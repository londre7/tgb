#include "tg_bot.h"

// потоки обработки обновлений
pthread_t                  *upd_thread = nullptr;            // потоки для обработки обновлений
std::queue<TGBOT_Update*>  *upd_thread_queue = nullptr;      // очередь для потоков-обработчиков
std::multiset<uint64_t>    *upd_thread_queue_uids = nullptr; // здесь будем кэшировать UID для каждого обновления
size_t                     *upd_queue_sizes = nullptr;       // длина каждой очереди
pthread_mutex_t            *upd_queue_mutex = nullptr;
pthread_mutex_t            *upd_queue_mutex_uids = nullptr;

int GetNumProcessors(void)
{
	#ifdef _SC_NPROCESSORS_ONLN
	return sysconf(_SC_NPROCESSORS_ONLN);
	#elif defined (PTW32_VERSION) 
	return pthread_num_processors_np();
	#else
	return 1;
	#endif
}

int GetNumUpdThreads(void)
{
	int val = GetBotConf()->GetIntParam(BotConfStruct::NumUpdateThreads);
	return (val > 0) ? val : GetNumProcessors();
}

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
	MySQLTablePtr tbl(QueryFromDB(SMAnsiString::smprintf("SELECT uid FROM tgb_find WHERE uid=%llu",uid)));
	if (tbl && tbl->Rows)
	{
		// обновляем
		return InsertToDB
		(
			SMAnsiString::smprintf
			(
				"UPDATE tgb_find SET start_find_time=%d WHERE uid=%llu",
				(int)time(NULL),
				uid
			)
		);
	}
	else
	{
		// добавляем
		return  InsertToDB
		(
			SMAnsiString::smprintf
			(
				"INSERT INTO tgb_find (uid, start_find_time) VALUES (%llu, %d)",
				uid,
				(int)time(NULL)
			)
		);
	}
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
			uint64_t recepient = usrlist_tbl->Cell[0][1];

			// создаём чат
			uint64_t innerID = CreateChat(uid, recepient);
			if(!innerID)
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);

			// берём первого попавшегося, ставим обоим состояние "чат" и в параметры ставим ID собеседника
			const char* params_str = "{\"chatid\":\"%llu\",\"innerid\":\"%llu\"}";
			if (!SetUserState(uid, USRSTATE_CHAT, SMAnsiString::smprintf(params_str, recepient, innerID)))
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);
			if (!SetUserState(recepient, USRSTATE_CHAT, SMAnsiString::smprintf(params_str, uid, innerID)))
				SEND_MSG_AND_CONTINUE(uid, BOTMSG_INTERNAL_ERROR);

			// отправляем обоим сообщения
			kb_cmd.CustomInit(true, false, true);
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
	DELETE_ARRAY_OBJECT(upd_thread_queue_uids);
	DELETE_ARRAY_OBJECT(upd_queue_sizes);
	DELETE_ARRAY_OBJECT(upd_queue_mutex);
	DELETE_ARRAY_OBJECT(upd_queue_mutex_uids);
}
void CreateUpdateThreads(int numthr)
{
	FreeUpdateTreads();

	upd_thread_queue = new std::queue<TGBOT_Update*>[numthr];
	upd_thread_queue_uids = new std::multiset<uint64_t>[numthr];
	upd_queue_sizes = new size_t[numthr];
	upd_thread = new pthread_t[numthr];
	upd_queue_mutex = new pthread_mutex_t[numthr];
	upd_queue_mutex_uids = new pthread_mutex_t[numthr];
	for (int i = 0, *idx; i < numthr; i++)
	{
		pthread_mutex_init(&upd_queue_mutex[i], NULL);
		pthread_mutex_init(&upd_queue_mutex_uids[i], NULL);
		idx = new int;
		*idx = i;
		pthread_create(&upd_thread[i], NULL, UpdThreadFunc, (void*)idx);
		pthread_detach(upd_thread[i]);
	}
}

bool PushToUpdQueue(TGBOT_Update *upd)
{
	if (!upd) return false;

	const int num_update_threads = GetNumUpdThreads();
	const uint64_t upd_chat_id = GetChatIDFromUpdate(upd);
	if (upd_chat_id != 0)
	{
		bool finded = false;
		// обновления от одного юзера помещаются в один поток
		for (int n = 0; n < num_update_threads; n++)
		{
			pthread_mutex_lock(&upd_queue_mutex_uids[n]);
			upd_queue_sizes[n] = upd_thread_queue_uids[n].size();
			if (!upd_queue_sizes[n])
			{
				pthread_mutex_unlock(&upd_queue_mutex_uids[n]);
				continue;
			}
			auto findel = upd_thread_queue_uids[n].find(upd_chat_id);
			if (findel != upd_thread_queue_uids[n].end())
			{
				// пихаем в эту очередь
				pthread_mutex_lock(&upd_queue_mutex[n]);
				upd_thread_queue[n].push(upd);
				upd_thread_queue_uids[n].insert(upd_chat_id);
				pthread_mutex_unlock(&upd_queue_mutex[n]);
				//WriteFormatMessage("Update %llu pushed to thread #%d", TGB_TEXTCOLOR_PURPURE, CurrentUpdate, n);
				finded = true;
				pthread_mutex_unlock(&upd_queue_mutex_uids[n]);
				break;
			}
			pthread_mutex_unlock(&upd_queue_mutex_uids[n]);
		}
		// если для этого юзера в очередях ничего нет, то поместим обновление в самый незагруженный поток
		if (!finded)
		{
			// в таком случае находим самый незагруженный поток
			size_t min_load_idx = 0;
			for (size_t n = 1, min = upd_queue_sizes[0]; n < num_update_threads; n++)
			{
				if (upd_queue_sizes[n] < min)
				{
					min = upd_queue_sizes[n];
					min_load_idx = n;
				}
			}

			pthread_mutex_lock(&upd_queue_mutex[min_load_idx]);
			pthread_mutex_lock(&upd_queue_mutex_uids[min_load_idx]);
			upd_thread_queue[min_load_idx].push(upd);
			upd_thread_queue_uids[min_load_idx].insert(upd_chat_id);
			pthread_mutex_unlock(&upd_queue_mutex[min_load_idx]);
			pthread_mutex_unlock(&upd_queue_mutex_uids[min_load_idx]);
			//WriteFormatMessage("Update %llu pushed to thread #%d", TGB_TEXTCOLOR_PURPURE, CurrentUpdate, min_load_idx);
		}
		return true;
	}
	else
		return false;
}



static TGBOT_Update* GetUpdateFromThreadQueue(int thrid)
{
	TGBOT_Update *update = nullptr;
	pthread_mutex_lock(&upd_queue_mutex[thrid]);
	if (!upd_thread_queue[thrid].empty())
	{
		update = upd_thread_queue[thrid].front();
		upd_thread_queue[thrid].pop();
		pthread_mutex_lock(&upd_queue_mutex_uids[thrid]);
		auto val = upd_thread_queue_uids[thrid].find(GetChatIDFromUpdate(update));
		if(val != upd_thread_queue_uids[thrid].end())
			upd_thread_queue_uids[thrid].erase(val);
		pthread_mutex_unlock(&upd_queue_mutex_uids[thrid]);
	}
	pthread_mutex_unlock(&upd_queue_mutex[thrid]);
	return update;
}

void* UpdThreadFunc(void *arg)
{
	int          thr_id=0;  // id потока
	DB_User      u_recv;
	
	// получаем id потока
	int *arg_i = (int*)arg;
	thr_id = *arg_i;
	DELETE_SINGLE_OBJECT(arg_i);

	// текущее время
	time_t curtime = time(NULL);

	// инициализируем генератор случайных чисел
	srand(unsigned(curtime+(time_t)thr_id));
	
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
		ProcessUpdate(update.get());
	}
	
	return NULL;
}