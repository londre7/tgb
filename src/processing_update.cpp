#include "tg_bot.h"

static int CheckUserExistInDB(DB_User& usr, TGBOT_User* recv_user, time_t message_time)
{
	MySQLTablePtr ret
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

	const size_t rows = ret->Rows;
	if (rows >= 1ull)
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
	return (int)rows;
}

static bool AddUserToDB(const DB_User& usr)
{
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"INSERT INTO tgb_users (uid, username, first_name, last_name, language_code, last_message_time) "
			"VALUES (%llu, \'%s\', \'%s\', \'%s\', \'%s\', %d)",
			usr.UID,
			C_STR(usr.Username),
			C_STR(usr.FirstName),
			C_STR(usr.LastName),
			C_STR(usr.LanguageCode),
			(int)usr.LastMessage
		)
	);
}

static void RunContact(TGBOT_Contact* contact, TGBOT_User* from, TGBOT_Chat* chat, DB_User& dbusrinfo)
{
	WriteFormatMessage(SYSTEMMSG_RECV_CONTACT, TGB_TEXTCOLOR_YELLOW, C_STR(contact->FirstName), C_STR(contact->PhoneNumber), C_STR(from->Username), C_STR(from->FirstName));
	if (contact->UserId == from->Id)
	{
		// пользователь прислал свой контакт, заносим в базу
		tgbot_SendMessage(from->Id, BOTMSG_RECR_USER_CONTACT);
		SetUserPhoneNumber(from->Id, contact->PhoneNumber);
	}
	else
		tgbot_SendMessage(from->Id, BOTMSG_RECR_OTHER_CONTACT);
}

static void RunSticker(TGBOT_Sticker* sticker, TGBOT_User* from, TGBOT_Chat* chat, DB_User& dbusrinfo)
{
	if (chat->Id == from->Id) // только из лички
	{
		const uint64_t uid = from->Id;
		switch (dbusrinfo.State)
		{
			case USRSTATE_FREE:
				tgbot_SendMessage(uid, BOTMSG_HELPMSG_FIND);
				break;
			case USRSTATE_FIND:
				tgbot_SendMessage(uid, BOTMSG_HELPMSG_CANCEL);
				break;
			case USRSTATE_CHAT:
			{
				GET_USRSTATE_PARAMS(pval, dbusrinfo.StateParams, USRSTATE_CHAT_params);
				uint64_t recepient = pval.at(0);
				if (recepient) tgbot_sendSticker(recepient, sticker->FileId);
			}
			break;
		}
		WriteFormatMessage(SYSTEMMSG_RECV_STICKER, TGB_TEXTCOLOR_YELLOW, C_STR(sticker->FileUniqueId), C_STR(from->Username), C_STR(from->FirstName));
	}
}

static void RunPhoto(TGBOT_ARRAY(TGBOT_PhotoSize) &photo, TGBOT_User* from, TGBOT_Chat* chat, DB_User& dbusrinfo)
{
	const size_t numphoto = photo.size();
	if (chat->Id == from->Id) // только из лички
	{
		WriteFormatMessage(SYSTEMMSG_RECV_PHOTO, TGB_TEXTCOLOR_YELLOW, numphoto, C_STR(from->Username), C_STR(from->FirstName));
	}
}

static void RunMessageProc(TGBOT_Message *message, DB_User &u_recv)
{
	if (message->Text == "")
	{
		// если не текстовое сообщение
		const size_t numphoto = message->Photo.size();

		if (message->Contact != NULL)      { RunContact(message->Contact, message->From, message->Chat, u_recv); }
		else if (message->Sticker != NULL) { RunSticker(message->Sticker, message->From, message->Chat, u_recv); }
		else if (numphoto)                 { RunPhoto(message->Photo, message->From, message->Chat, u_recv);     }
		else
			WriteFormatMessage(SYSTEMMSG_RECV_NONTEXTMSG, TGB_TEXTCOLOR_YELLOW, C_STR(message->From->Username), C_STR(message->From->FirstName));
	}
	else
	{
		if (message->From->Id == message->Chat->Id) // только из лички
		{
			WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_RECV_MESSAGE, C_STR(message->From->Username), C_STR(message->From->FirstName), C_STR(message->Text)));
			SMAnsiString recv_cmd = message->Text;
			if (recv_cmd[0] != '/')
			{
				// не команда
				fm_processing(u_recv, message->From, message->Chat, message);
			}
			else
			{
				// готовим параметры
				const char* ptrb = "";
				int kdel = recv_cmd.Pos(' ');
				if (kdel > 0)
				{
					recv_cmd[kdel] = '\0';
					ptrb = &recv_cmd[kdel + 1];
					// если первые символы пробелы - перемещаем указатель до первого символа
					while (*ptrb == ' ') { ptrb++; }
				}

				// выполняем команду
				RunProcCmd(recv_cmd, ptrb, message, u_recv);
			}
		}
	}
}

static void RunEditedMessageProc(TGBOT_Message* edited_message, DB_User& u_recv)
{
	WriteMessage
	(
		SMAnsiString::smprintf
		(
			SYSTEMMSG_RECV_EDITED_MESSAGE,
			edited_message->MessageId,
			C_STR(edited_message->Chat->FirstName),
			C_STR(edited_message->From->Username),
			C_STR(edited_message->From->FirstName),
			C_STR(edited_message->Text)
		)
	);
}

void ProcessUpdate(TGBOT_Update* upd)
{
	int state_before;
	SMAnsiString state_before_params;
	time_t curtime = time(NULL);
	
	DB_User u_recv;
	TGBOT_User *user = GetUserFromUpdate(upd);
	int user_check_result = CheckUserExistInDB(u_recv, user, curtime);
	if (user_check_result == -1)
	{
		WriteMessage(SYSTEMMSG_UPD_USERINFO_FAILED, TGB_TEXTCOLOR_RED);
		SaveLastBotState(STATE_FILE_FULL_PATH);
		return;
	}

	// запоминаем состояние
	state_before = u_recv.State;
	state_before_params = u_recv.StateParams;

	// обрабатываем
	if (upd->CallbackQuery != nullptr) { RunCallbackProc(upd->CallbackQuery, u_recv);      }
	if (upd->Message != nullptr)       { RunMessageProc(upd->Message, u_recv);             }
	if (upd->EditedMessage != nullptr) { RunEditedMessageProc(upd->EditedMessage, u_recv); }

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
				WriteFormatMessage(SYSTEMMSG_REGISTER_USR_SUCCESS, TGB_TEXTCOLOR_LIGHTBLUE, C_STR(usr.Username), C_STR(usr.FirstName), C_STR(usr.LastName), C_STR(usr.LanguageCode));
			}
		}
		else
		{
			InsertToDB
			(
				((usr.State == state_before) && (usr.StateParams == state_before_params)) ?
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
				) :
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
}