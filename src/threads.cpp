#include "tg_bot.h"

// потоки обработки обновлений
pthread_t                  *upd_thread = nullptr;       // потоки для обработки обновлений
std::vector<TGBOT_Update*> *upd_thread_queue = nullptr; // очередь для потоков-обработчиков
int                        *upd_queue_sizes = nullptr;  // длина каждой очереди
pthread_mutex_t            *upd_queue_mutex = nullptr;

int GetNumUpdThreads(void) { return GetBotConf()->GetIntParam(BotConfStruct::NumUpdateThreads); }

// поисковый поток
static FindThread g_findThread;
FindThread* GetFindThread() { return &g_findThread; }

FindThread::FindThread()
{
	mutex_init();
	pthread_create(&thrpar.m_Thr, nullptr, thrFunc, (void*)& thrpar);
	pthread_detach(thrpar.m_Thr);
}

void FindThread::StartFind(uint64_t uid)
{
	pthread_mutex_lock(&thrpar.m_UsrMutex);
	thrpar.m_Users.AddKey(uid);
	pthread_mutex_unlock(&thrpar.m_UsrMutex);
}
inline void FindThread::mutex_init()
{
	thrpar.m_UsrMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&thrpar.m_UsrMutex, nullptr);
}
static bool SetStartFindTime(uint64_t uid)
{
	bool b = InsertToDB
	(
		SMAnsiString::smprintf
		(
			"INSERT INTO tgb_find (uid, start_find_time) VALUES (%llu, %d)",
			uid,
			(int)time(NULL)
		)
	);
	if (!b)
	{
		b = InsertToDB
		(
			SMAnsiString::smprintf
			(
				"UPDATE tgb_find SET start_find_time=%d WHERE uid=%llu",
				(int)time(NULL),
				uid
			)
		);
	}
	return b;
}
void* FindThread::thrFunc(void* arg)
{
#define SEND_MSG_AND_CONTINUE(uid,msg) {tgbot_SendMessage(uid,msg);continue;}
	thr_par* thrpar = (thr_par*)arg;
	while (true)
	{
		if (thrpar->m_Users.GetNumElem())
		{
			// достаём юзера
			pthread_mutex_lock(&thrpar->m_UsrMutex);
			const uint64_t uid = thrpar->m_Users.RemoveKeyByIndex(0);
			pthread_mutex_unlock(&thrpar->m_UsrMutex);
			WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_FINDTHR_START_FIND, uid), TGB_TEXTCOLOR_PURPURE);

			// запрашиваем пользователей в состянии "поиск"
			MySQLTablePtr usrlist_tbl
			(
				QueryFromDB
				(
					SMAnsiString::smprintf
					(
						"SELECT tgb_users.id, tgb_users.uid, tgb_users.username, tgb_users.first_name, tgb_users.last_name, tgb_users.state, tgb_find.start_find_time "
						"FROM tgb_users LEFT JOIN tgb_find ON tgb_users.uid=tgb_find.uid "
						"WHERE tgb_users.state=1 AND tgb_users.uid<>%llu ORDER BY tgb_find.start_find_time ASC",
						uid
					)
				)
			);
			if (!usrlist_tbl)
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);

			if (!SetStartFindTime(uid))
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);

			TGBOT_ReplyKeyboardMarkup kb_cmd(true, false, true);
			kb_cmd.CreateButton(REPLYBTN_CAPTION_CANCEL);
			tgbot_SendMessage(uid, BOTMSG_START_FIND, &kb_cmd);

			const size_t numline = usrlist_tbl->Rows;
			if (!numline)
			{
				// переводим юзера в режим поиска
				SetUserState(uid, USRSTATE_FIND, "");
				continue;
			}

			// берём первого попавшегося, ставим обоим состояние "чат" и в параметры ставим ID собеседника
			uint64_t recepient = usrlist_tbl->Cell[0][1];
			const char* params_str = "{\"chatid\":\"%llu\"}";
			if (!SetUserState(uid, USRSTATE_CHAT, SMAnsiString::smprintf(params_str, recepient)))
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);
			if (!SetUserState(recepient, USRSTATE_CHAT, SMAnsiString::smprintf(params_str, uid)))
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);

			// отправляем обоим сообщения
			kb_cmd.Init(true, false, true);
			kb_cmd.CreateButton(REPLYBTN_CAPTION_STOP);
			tgbot_SendMessage(uid, BOTMSG_START_CHAT, &kb_cmd);
			tgbot_SendMessage(recepient, BOTMSG_START_CHAT, &kb_cmd);
			WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_FINDTHR_START_CHAT, uid, recepient), TGB_TEXTCOLOR_PURPURE);
		}
		else usleep(10000);
	}
	return nullptr;
#undef SEND_MSG_AND_CONTINUE
}

inline void FreeUpdateTreads()
{
	DELETE_ARRAY_OBJECT(upd_thread);
	DELETE_ARRAY_OBJECT(upd_thread_queue);
	DELETE_ARRAY_OBJECT(upd_queue_sizes);
	DELETE_ARRAY_OBJECT(upd_queue_mutex);
}
void CreateUpdateThreads(int numthr)
{
	FreeUpdateTreads();

	upd_thread_queue = new std::vector<TGBOT_Update*>[numthr];
	upd_queue_sizes = new int[numthr];
	upd_thread = new pthread_t[numthr];
	upd_queue_mutex = new pthread_mutex_t[numthr];
	for (int i = 0, *idx; i < numthr; i++)
	{
		pthread_mutex_init(&upd_queue_mutex[i], NULL);
		idx = new int;
		*idx = i;
		pthread_create(&upd_thread[i], NULL, UpdThreadFunc, (void*)idx);
		pthread_detach(upd_thread[i]);
	}
}

bool PushToUpdQueue(TGBOT_Update* upd)
{
	if (!upd) return false;

	const int num_update_threads = GetNumUpdThreads();
	const uint64_t upd_chat_id = GetChatIDFromUpdate(upd);
	if (upd_chat_id != 0)
	{
		bool finded = false;
		for (int n = 0, breakflag = 0; n < num_update_threads; n++)
		{
			upd_queue_sizes[n] = upd_thread_queue[n].size();
			for (int m = 0; m < upd_queue_sizes[n]; m++)
			{
				if (GetChatIDFromUpdate(upd_thread_queue[n][m]) == upd_chat_id)
				{
					// пихаем в эту очередь
					pthread_mutex_lock(&upd_queue_mutex[n]);
					upd_thread_queue[n].push_back(upd);
					pthread_mutex_unlock(&upd_queue_mutex[n]);
					//WriteFormatMessage("Update %llu pushed to thread #%d", TGB_TEXTCOLOR_PURPURE, CurrentUpdate, n);

					breakflag = 1;
					finded = true;
					break;
				}
			}
			if (breakflag) break;
		}
		if (!finded)
		{
			// в таком случае находим самый незагруженный поток
			int min_load_idx = 0;
			for (int n = 1, min = upd_queue_sizes[0]; n < num_update_threads; n++)
			{
				if (upd_queue_sizes[n] < min)
				{
					min = upd_queue_sizes[n];
					min_load_idx = n;
				}
			}
			pthread_mutex_lock(&upd_queue_mutex[min_load_idx]);
			upd_thread_queue[min_load_idx].push_back(upd);
			pthread_mutex_unlock(&upd_queue_mutex[min_load_idx]);
			//WriteFormatMessage("Update %llu pushed to thread #%d", TGB_TEXTCOLOR_PURPURE, CurrentUpdate, min_load_idx);
		}
		return true;
	}
	else
		return false;
}

static int CheckUserExistInDB(DB_User& usr, TGBOT_User* recv_user, time_t message_time)
{
	std::unique_ptr<SMMYSQL_Table> ret
	(
		QueryFromDB
		(
			SMAnsiString::smprintf("SELECT * FROM tgb_users WHERE uid=%llu", recv_user->Id)
		)
	);
	if (ret == NULL)
		return -1;
	
	usr.UID = recv_user->Id;
	usr.Username = recv_user->Username;
	usr.FirstName = recv_user->FirstName;
	usr.LastName = recv_user->LastName;
	usr.LanguageCode = recv_user->LanguageCode;

	const int rows = ret->Rows;
	if (rows >= 1)
	{
		usr.LastMessage = ret->Cell[0][6];
		usr.PhoneNumber = ret->Cell[0][7];
		usr.Permissions = ret->Cell[0][8];
		usr.Notify = ret->Cell[0][9];
		usr.State = ret->Cell[0][10];
		usr.StateParams = ret->Cell[0][11];
	}
	else
	{
		usr.LastMessage = message_time;
	}
	return rows;
}

static bool AddUserToDB(const DB_User& usr)
{
	if (!InsertToDB("INSERT INTO tgb_users (uid, username, first_name, last_name, language_code, last_message_time) VALUES (" + SMAnsiString(usr.UID) + ", \'" + usr.Username + "\', \'" + usr.FirstName + "\', \'" + usr.LastName + "\', \'" + usr.LanguageCode + "\', " + SMAnsiString(usr.LastMessage) + ")"))
		return false;
	return true;
}

static TGBOT_Update* GetUpdateFromThreadQueue(int thrid)
{
	TGBOT_Update* update = nullptr;
	pthread_mutex_lock(&upd_queue_mutex[thrid]);
	if (!upd_thread_queue[thrid].empty()) update = upd_thread_queue[thrid][0];
	pthread_mutex_unlock(&upd_queue_mutex[thrid]);
	return update;
}

void* UpdThreadFunc(void *arg)
{
	int          thr_id=0;  // id потока
	DB_User      u_recv;
	int          kdel;
	SMAnsiString cmd,
	             param,
	             recv_cmd;
	MYSQL_RES    *res;
	
	// получаем id потока
	int *arg_i = (int*)arg;
	thr_id = *arg_i;
	DELETE_SINGLE_OBJECT(arg_i);

	// текущее время
	time_t curtime = time(NULL);

	// инициализируем генератор случайных чисел
	srand(curtime+thr_id);
	
	while(true)
	{
		// получаем обновление и обрабатываем его
		std::unique_ptr<TGBOT_Update> update(GetUpdateFromThreadQueue(thr_id));
		
		if(update == NULL)
		{
			usleep(500);
			continue;
		}
		
		// обрабатываем обновление
		int user_check_result = -1;
		int state_before;
		SMAnsiString state_before_params;
		if(update->CallbackQuery != NULL)
		{
			TGBOT_CallbackQuery *callback_query = update->CallbackQuery;
			user_check_result = CheckUserExistInDB(u_recv, callback_query->From, curtime);
			if (user_check_result == -1)
			{
				WriteMessage(SYSTEMMSG_UPD_USERINFO_FAILED, TGB_TEXTCOLOR_RED);
				SaveLastBotState(STATE_FILE_FULL_PATH);
				continue;
			}
			state_before = u_recv.State;
			state_before_params = u_recv.StateParams;

			WriteFormatMessage(SYSTEMMSG_RECV_CALLBACK_QUERY, TGB_TEXTCOLOR_YELLOW, C_STR(callback_query->Data), C_STR(callback_query->From->Username), C_STR(callback_query->From->FirstName));
			RunCallbackProc(callback_query, u_recv);
		}
			
		if(update->Message != NULL)
		{
			TGBOT_Message *message = update->Message; // ссылка для обратной совместимости
				
			// проверяем наличие юзера в базе, если есть обновляем о нём инфу
			user_check_result = CheckUserExistInDB(u_recv, message->From, message->Date);
			if (user_check_result == -1)
			{
				WriteMessage(SYSTEMMSG_UPD_USERINFO_FAILED, TGB_TEXTCOLOR_RED);
				SaveLastBotState(STATE_FILE_FULL_PATH);
				continue;
			}
			state_before = u_recv.State;
			state_before_params = u_recv.StateParams;
				
			if(message->Text == "")
			{
				// если не текстовое сообщение
				if(message->Contact != NULL)
				{
					WriteFormatMessage(SYSTEMMSG_RECV_CONTACT, TGB_TEXTCOLOR_YELLOW, C_STR(message->Contact->FirstName), C_STR(message->Contact->PhoneNumber), C_STR(message->From->Username), C_STR(message->From->FirstName));
					if(message->Contact->UserId == message->From->Id)
					{
						// пользователь прислал свой контакт, заносим в базу
						tgbot_SendMessage(message->From->Id, BOTMSG_RECR_USER_CONTACT);
						SetUserPhoneNumber(message->From->Id, message->Contact->PhoneNumber);
					}
					else
						tgbot_SendMessage(message->From->Id, BOTMSG_RECR_OTHER_CONTACT);
				}
				else if(message->Sticker != NULL) // Может, это стикер? о_О
				{
					if (message->Chat->Id == message->From->Id) // только из лички
					{
						const uint64_t uid = message->From->Id;
						switch (u_recv.State)
						{
							case USRSTATE_FREE:
								tgbot_SendMessage(uid, BOTMSG_HELPMSG_FIND);
								break;
							case USRSTATE_FIND:
								tgbot_SendMessage(uid, BOTMSG_HELPMSG_CANCEL);
								break;
							case USRSTATE_CHAT:
								{
									GET_USRSTATE_PARAMS(u_recv.StateParams, USRSTATE_CHAT_params);
									uint64_t recepient = gupvalues.at(0);
									if (recepient) tgbot_sendSticker(recepient, message->Sticker->FileId);
								}
								break;
						}
						WriteFormatMessage(SYSTEMMSG_RECV_STICKER, TGB_TEXTCOLOR_YELLOW, C_STR(message->Sticker->FileUniqueId), C_STR(message->From->Username), C_STR(message->From->FirstName));
					}
				}
				else if(message->NumPhoto != 0) // да тут могут быть фотки о_@
				{
					if (message->Chat->Id == message->From->Id) // только из лички
					{
						WriteFormatMessage(SYSTEMMSG_RECV_PHOTO, TGB_TEXTCOLOR_YELLOW, message->NumPhoto, C_STR(message->From->Username), C_STR(message->From->FirstName));
					}
				}
				else
					WriteFormatMessage(SYSTEMMSG_RECV_NONTEXTMSG, TGB_TEXTCOLOR_YELLOW, C_STR(message->From->Username), C_STR(message->From->FirstName));
			}
			else
			{
				if (message->From->Id == message->Chat->Id) // только из лички
				{
					WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_RECV_MESSAGE, C_STR(message->From->Username), C_STR(message->From->FirstName), C_STR(message->Text)));
					recv_cmd = message->Text;
					if(recv_cmd[0] != '/')
					{
						// не команда
						fm_processing(u_recv, message->From, message->Chat, recv_cmd, message->Message_Id);
					}
					else
					{
						// готовим параметры
						kdel = recv_cmd.Pos(' ');
						if(kdel > 0) cmd = recv_cmd.Delete(kdel, recv_cmd.length());
						else cmd = recv_cmd;
						
						if(kdel > 0)
						{
							param = recv_cmd;
							param = param.Delete(0, kdel+1);
							for(int i=0; i<param.length(); i++)
							{
								if(i == 0)
								{
									if(param[i] != ' ') break;
								}				
								else
								{
									if(param[i] != ' ')
									{
										param = param.Delete(0, i);
										break;
									}
								}
							}
						}
						else param = "";
						
						// выполняем команду
						RunProcCmd(cmd, param, message, u_recv);
					}
				}
			}
		}
			
		// проверяем, есть ли редачеры
		if(update->EditedMessage != NULL)
		{
			// отредактированное сообщение
			TGBOT_Message *edited_message = update->EditedMessage;
			WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_RECV_EDITED_MESSAGE, edited_message->Message_Id, C_STR(edited_message->Chat->FirstName), C_STR(edited_message->From->Username), C_STR(edited_message->From->FirstName), C_STR(edited_message->Text)));
			//if(edited_message->From->Id == edited_message->Chat->Id) tgbot_SendMessage(edited_message->From->Id, "Редачер!");
		}

		// оновляем инфу по юзеру
		auto UpdateUserInDB = [&user_check_result, &state_before, &state_before_params](DB_User & usr)
		{
			usr.LastMessage = time(NULL);

			// проверяем наличие юзера в базе, если есть обновляем о нём инфу
			if (user_check_result == -1)
				return false;
			else if (user_check_result == 0)
			{
				// создаём юзера
				if (!AddUserToDB(usr))
				{
					WriteMessage(SYSTEMMSG_REGISTER_USR_FAILED, TGB_TEXTCOLOR_RED);
					return false;
				}
				else
				{
					std::unique_ptr<SMKeyList> usr_list(GetUsersListWithNotifyMask(NOTIFY_REGISTER_NEW_USER));
					tgbot_SendMessage(usr_list.get(), SMAnsiString::smprintf(BOTMSG_REGISTER_NEW_USER, C_STR(usr.InfoStr())));
					WriteFormatMessage(SYSTEMMSG_REGISTER_USR_SUCCESS, TGB_TEXTCOLOR_LIGHTBLUE, C_STR(usr.Username), C_STR(usr.FirstName), C_STR(usr.LastName), C_STR(usr.LanguageCode));
				}
			}
			else
			{
				InsertToDB
				(
					((usr.State == state_before)&&(usr.StateParams == state_before_params))?
					SMAnsiString::smprintf
					(
						"UPDATE tgb_users SET last_message_time=%d, username='%s', first_name='%s', last_name='%s', language_code='%s', phone_number='%s', access=%llu, notify=%llu WHERE uid=%llu",
						(int)usr.LastMessage,
						C_STR(usr.Username),
						C_STR(usr.FirstName),
						C_STR(usr.LastName),
						C_STR(usr.LanguageCode),
						C_STR(usr.PhoneNumber),
						usr.Permissions,
						usr.Notify,
						usr.UID
					):
					SMAnsiString::smprintf
					(
						"UPDATE tgb_users SET last_message_time=%d, username='%s', first_name='%s', last_name='%s', language_code='%s', phone_number='%s', access=%llu, notify=%llu, state=%d, state_params='%s' WHERE uid=%llu",
						(int)usr.LastMessage,
						C_STR(usr.Username),
						C_STR(usr.FirstName),
						C_STR(usr.LastName),
						C_STR(usr.LanguageCode),
						C_STR(usr.PhoneNumber),
						usr.Permissions,
						usr.Notify,
						usr.State,
						C_STR(usr.StateParams),
						usr.UID
					)
				);
			}
			return true;
		};
		UpdateUserInDB(u_recv);
		
		// удаляем из очереди
		pthread_mutex_lock(&upd_queue_mutex[thr_id]);
		upd_thread_queue[thr_id].erase(upd_thread_queue[thr_id].begin());
		pthread_mutex_unlock(&upd_queue_mutex[thr_id]);
	}
	
	return NULL;
}