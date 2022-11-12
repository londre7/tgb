// здесь описываем обработчики для команд "/"
#include "tg_bot.h"

// список команд и их обработчики
typedef void (*cmd_proc_func)(DB_User&, TGBOT_User*, TGBOT_Chat*, const StringList*, uint64_t);
struct Cmd
{ 
	const char* cmd;
	const char* replybtn;
	cmd_proc_func proc;
	int usrstate;
	uint64_t permission;
	bool and_flag; // если true, доступ только когда все флажки из permission стоят у пользователя, если false - хотя бы один.
} 
CmdDef[] =
{
	// команда           соотв. кнопка            обработчик                    необх. сост.   необходимые разрешения        флаг "И"  
	{ "/start",          nullptr,                 sc_processing_start,          USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/find",           REPLYBTN_CAPTION_FIND,   sc_processing_find,           USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/cancel",         REPLYBTN_CAPTION_CANCEL, sc_processing_cancel,         USRSTATE_ALL,  DEFAULT_USER_PERMISIONS,      false },
	{ "/stop",           REPLYBTN_CAPTION_STOP,   sc_processing_stop,           USRSTATE_CHAT, DEFAULT_USER_PERMISIONS,      false },
	{ "/ap",             nullptr,                 sc_processing_ap,             USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/getpermissions", nullptr,                 sc_processing_getpermissions, USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/setpermissions", nullptr,                 sc_processing_setpermissions, USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/getuserinfo",    nullptr,                 sc_processing_getuserinfo,    USRSTATE_FREE, FLAGS_0_1,                    false },
	{ "/getnotify",      nullptr,                 sc_processing_getnotify,      USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/setnotify",      nullptr,                 sc_processing_setnotify,      USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
};

static bool CheckUserPermission(uint64_t cmdPermissions, const DB_User &user, bool and_flag)
{
	uint64_t flag = user.Permissions & cmdPermissions;
	bool successfully = (and_flag) ? (flag == cmdPermissions) : flag;
	bool accessed = ((cmdPermissions > 0ull) && !successfully) ? false : true;
	return (accessed || IsSuperUser(user.Username));
}

static const char* CheckUserState(int cmd_usrstate, const DB_User& user)
{
	const char *EMPTYSTR = "";
	// здесь объявляем кастомные сообщения, которые будут выводиться, если пользователь не в нужном состянии
	static std::map<int, const char*> msgMap =
	{
		{ USRSTATE_CHAT, BOTMSG_OUTSIDE_CHAT }
	};

	if (cmd_usrstate == USRSTATE_ALL) return EMPTYSTR;
	if (cmd_usrstate != user.State)
	{
		const char* ret_msg = BOTMSG_USR_NO_STATE_FREE;
		try
		{
			ret_msg = msgMap.at(cmd_usrstate);
		}
		catch (...) {}
		return ret_msg;
	}
	else
		return EMPTYSTR;
}

static void run_cmd(const Cmd &cmd, DB_User &dbusrinfo, const SMAnsiString& param, TGBOT_Message* message)
{
	// проверяем разрешения
	if (!CheckUserPermission(cmd.permission, dbusrinfo, cmd.and_flag))
		PERMISSION_DENIED(message->Chat->Id);

	// проверяем необходимое состяние
	const char* err_msg = CheckUserState(cmd.usrstate, dbusrinfo);
	if (!IsStrEmpty(err_msg))
		SEND_MSG_AND_RETURN(message->Chat->Id, err_msg);

	// выполняем команду
	std::unique_ptr<StringList> params(ParseFormatString(param));
	cmd.proc(dbusrinfo, message->From, message->Chat, params.get(), message->Message_Id);
	return;
}

void RunProcCmd(const SMAnsiString& cmd, const SMAnsiString& param, TGBOT_Message* message, DB_User& dbusrinfo)
{
	// выполняем команду
	for (auto& command : CmdDef)
	{
		if (cmd == command.cmd)
			return run_cmd(command, dbusrinfo, param, message);
	}
	// неизвестная команда
	sc_processing_unknown(dbusrinfo, message->From, message->Chat, param, message->Message_Id);
}

void sc_processing_unknown(DB_User& dbusrinfo, TGBOT_User *RecvUser, TGBOT_Chat *RecvChat, SMAnsiString Params, uint64_t MessageID)
{
	tgbot_SendMessage(RecvChat->Id, BOTMSG_CMD_UNKNOWN);
}

void sc_processing_start(DB_User& dbusrinfo, TGBOT_User *RecvUser, TGBOT_Chat *RecvChat, const StringList* Params, uint64_t MessageID)
{
	// клавиатура, чисто для примера
	static std::vector<InlineKeyboardDef> kb_start =
	{
		{ INLINEBTN_CAPTION_PRIVATE_POLICY, CALLBACK_PRIVATE_POLICY, nullptr },
		//{ "#newrow",						nullptr,                 nullptr },
		//{ INLINEBTN_CAPTION_ABOUT,          CALLBACK_ABOUT,          &CallbackParamsDef_About }
	};

	TGBOT_ReplyKeyboardMarkup kb_cmd(true, false, true);
	kb_cmd.CreateButton(REPLYBTN_CAPTION_FIND);

	// параметры для callback
	StringList valuesAbout = { "param" };
	std::unique_ptr<TGBOT_InlineKeyboardMarkup> kb(MakeInlineKeyboardFromDef(kb_start, {nullptr,&valuesAbout}));

	// посылаем сообщенние с клавой
	SMAnsiString text;
	text.smprintf_s(BOTMSG_CMD_START, C_STR(MakeFullUserName(RecvUser)));
	tgbot_SendMessage(RecvChat->Id, text, &kb_cmd);
}

void sc_processing_find(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	// уводим поиск в отдельный поток
	GetFindThread()->StartFind(dbusrinfo.UID);
}

void sc_processing_cancel(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	// общая команда, обработаем в зависимости от состояния пользователя
	auto RESET_STATE = [](DB_User &dbusrinfo, uint64_t chatId, const char *message)
	{
		dbusrinfo.State = USRSTATE_FREE;
		dbusrinfo.StateParams = "";
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, message, REPLYBTN_CAPTION_FIND);
	};

	switch (dbusrinfo.State)
	{
		case USRSTATE_FREE:
			tgbot_SendMessage(RecvChat->Id, BOTMSG_NOTHING_CANCEL);
			break;
		case USRSTATE_FIND:
			// отменяем поиск
			RESET_STATE(dbusrinfo, RecvChat->Id, BOTMSG_FIND_CANCEL);
			break;
		case USRSTATE_SETPERMISSION_INPUT_UID:
		case USRSTATE_SETPERMISSION_INPUT_NBIT:
		case USRSTATE_SETPERMISSION_INPUT_VALUE:
		case USRSTATE_GETPERMISSION_INPUT_UID:
		case USRSTATE_GETUSERINFO_INPUT_UID:
		case USRSTATE_GETNOTIFY_INPUT_UID:
		case USRSTATE_SETNOTIFY_INPUT_UID:
			RESET_STATE(dbusrinfo, RecvChat->Id, BOTMSG_CANCEL);
			break;
		default:
			tgbot_SendMessage(RecvChat->Id, BOTMSG_USR_NO_STATE_FREE);
			break;
	}	
}

void sc_processing_stop(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	// достаём ID собеседника
	GET_USRSTATE_PARAMS(dbusrinfo.StateParams, USRSTATE_CHAT_params);
	uint64_t recepient = gupvalues.at(0);

	// завершаем диалог
	if (!SetUserState(dbusrinfo, USRSTATE_FREE, ""))
		SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_INTERNAL_ERROR);
	if (!SetUserState(recepient, USRSTATE_FREE, ""))
		SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_INTERNAL_ERROR);

	// выводим сообщение
	TGBOT_ReplyKeyboardMarkup kb_cmd(true, false, true);
	kb_cmd.CreateButton(REPLYBTN_CAPTION_FIND);
	tgbot_SendMessage(RecvChat->Id, BOTMSG_STOP_CHAT_BY_ME, &kb_cmd);
	tgbot_SendMessage(recepient, BOTMSG_STOP_CHAT_BY_RECEPIENT, &kb_cmd);
}

void sc_processing_ap(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	/*static std::vector<InlineKeyboardDef> adm_menu =
	{
		{ INLINEBTN_CAPTION_PRIVATE_POLICY, CALLBACK_PRIVATE_POLICY, nullptr },
		//{ "#newrow",						nullptr,                 nullptr },
		//{ INLINEBTN_CAPTION_ABOUT,          CALLBACK_ABOUT,          &CallbackParamsDef_About }
	};

	// параметры для callback
	StringList valuesAbout = { "param" };
	std::unique_ptr<TGBOT_InlineKeyboardMarkup> kb(MakeInlineKeyboardFromDef(adm_menu, { nullptr,&valuesAbout }));*/
	tgbot_SendMessage(RecvChat->Id, BOTMSG_IN_DEVELOP);
}

static void proc_getflag_cmd(uint64_t flag, const StringList* str, const SMAnsiString& helpmsg, const StringList* Params,
	uint64_t chatId, DB_User& dbusrinfo, int to_state)
{
	// достаём параметры
	const size_t numparam = Params ? Params->size() : 0;
	if (!numparam)
	{
		// переводим в интерактивный режим
		dbusrinfo.State = to_state;
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
	}
	if (numparam != 1)
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	const SMAnsiString & target_user = Params->at(0);
	std::unique_ptr<DB_User> usr((target_user.IsValidNumber()) ? GetUserByUID(target_user) : GetUserByUsername(target_user));
	if (!usr)
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_USER_NOT_FOUND, REPLYBTN_CAPTION_FIND);

	SEND_MSG_AND_RETURN_WITH_BTN(chatId, MakePermissionString(flag, str), REPLYBTN_CAPTION_FIND);
}

static void proc_setflag_cmd(uint64_t &flag, const SMAnsiString& helpmsg, const StringList* Params,
	uint64_t chatId, DB_User& dbusrinfo, int to_state)
{
	// проверяем количество параметров
	const size_t numparam = Params ? Params->size() : 0;
	if (!numparam)
	{
		// переводим в интерактивный режим
		dbusrinfo.State = to_state;
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
	}
	if (numparam != 3)
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	// достаём параметры
	const SMAnsiString & target_user = Params->at(0);
	const SMAnsiString & nbit_str = Params->at(1);
	const SMAnsiString & setval_str = Params->at(2);
	const bool IsUID = target_user.IsValidNumber();

	// валидация параметров
	if (!nbit_str.IsValidNumber())
		SEND_MSG_AND_RETURN(chatId, helpmsg);
	if (!setval_str.IsValidNumber())
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	const unsigned int nbit = nbit_str;
	const unsigned int setval = setval_str;

	// ещё одна валидация
	const int masksz = sizeof(uint64_t) * 8;
	if ((nbit < 1) || (nbit > 64))
		SEND_MSG_AND_RETURN(chatId, helpmsg);
	if (setval > 1)
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	// самому себе трогать первый флажок нельзя
	const bool byMyself = (IsUID) ? (dbusrinfo.UID == uint64_t(target_user)) : (dbusrinfo.Username == target_user);
	if ((to_state == USRSTATE_SETPERMISSION_INPUT_UID) && (nbit == 1) && byMyself)
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_DENIED_MYSELF, REPLYBTN_CAPTION_FIND);

	// достаём пользователя
	std::unique_ptr<DB_User> usr((IsUID) ? GetUserByUID(target_user) : GetUserByUsername(target_user));
	if (!usr)
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_USER_NOT_FOUND, REPLYBTN_CAPTION_FIND);

	// ставим права
	uint64_t _before_p = flag;
	if (setval) SetBit64(flag, nbit - 1);
	else UnsetBit64(flag, nbit - 1);
	if (_before_p == flag)
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_SUCCESSFULL, REPLYBTN_CAPTION_FIND);
	if (!SetUserPermission(usr->UID, flag))
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_INTERNAL_ERROR, REPLYBTN_CAPTION_FIND);

	SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_SUCCESSFULL, REPLYBTN_CAPTION_FIND);
}

void sc_processing_getpermissions(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helpmsg;
	{
		static const char* helpmsg_syntax = "<i>/getpermissions</i> [uid]|[username]";
		static const char* helpmsg_example = "<i>/getpermissions</i> 123456\n<i>/getpermissions</i> dummyuser";
		helpmsg.smprintf_s(BOTMSG_CMD_INVALID_PARAMS, helpmsg_syntax, helpmsg_example);
	}
	proc_getflag_cmd(dbusrinfo.Permissions, GetUsrAccessStrList(), helpmsg, Params, RecvChat->Id, dbusrinfo, USRSTATE_GETPERMISSION_INPUT_UID);
}

void sc_processing_setpermissions(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helpmsg;
	{
		static const char* helpmsg_syntax = "<i>/setpermissions</i> [uid]|[username] [nbit] [0|1]";
		static const char* helpmsg_example = "<i>/setpermissions</i> 123456 3 1\n<i>/setpermissions</i> dummyuser 2 0";
		helpmsg.smprintf_s(BOTMSG_CMD_INVALID_PARAMS, helpmsg_syntax, helpmsg_example);
	}
	proc_setflag_cmd(dbusrinfo.Permissions, helpmsg, Params, RecvChat->Id, dbusrinfo, USRSTATE_SETPERMISSION_INPUT_UID);
}

void sc_processing_getuserinfo(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helpmsg;
	{
		static const char* helpmsg_syntax = "<i>/getuserinfo</i> [uid]|[username]";
		static const char* helpmsg_example = "<i>/getuserinfo</i> 123456\n<i>/setpermissions</i> dummyuser";
		helpmsg.smprintf_s(BOTMSG_CMD_INVALID_PARAMS, helpmsg_syntax, helpmsg_example);
	}

	// проверяем количество параметров
	const size_t numparam = Params ? Params->size() : 0;
	if (!numparam)
	{
		// переводим в интерактивный режим
		dbusrinfo.State = USRSTATE_GETUSERINFO_INPUT_UID;
		SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
	}
	if (numparam != 1)
		SEND_MSG_AND_RETURN(RecvChat->Id, helpmsg);

	// достаём юзера
	const SMAnsiString& target_user = Params->at(0);
	std::unique_ptr<DB_User> usr((target_user.IsValidNumber()) ? GetUserByUID(target_user) : GetUserByUsername(target_user));
	if (!usr)
		SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, BOTMSG_USER_NOT_FOUND, REPLYBTN_CAPTION_FIND);

	SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, usr->InfoStr(), REPLYBTN_CAPTION_FIND);
}

void sc_processing_getnotify(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helpmsg;
	{
		static const char* helpmsg_syntax = "<i>/getnotify</i> [uid]|[username]";
		static const char* helpmsg_example = "<i>/getnotify</i> 123456\n<i>/getnotify</i> dummyuser";
		helpmsg.smprintf_s(BOTMSG_CMD_INVALID_PARAMS, helpmsg_syntax, helpmsg_example);
	}
	proc_getflag_cmd(dbusrinfo.Notify, GetUsrNotifyStrList(), helpmsg, Params, RecvChat->Id, dbusrinfo, USRSTATE_GETNOTIFY_INPUT_UID);
}

void sc_processing_setnotify(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helpmsg;
	{
		static const char* helpmsg_syntax = "<i>/setnotify</i> [uid]|[username] [nbit] [0|1]";
		static const char* helpmsg_example = "<i>/setnotify</i> 123456 3 1\n<i>/setnotify</i> dummyuser 2 0";
		helpmsg.smprintf_s(BOTMSG_CMD_INVALID_PARAMS, helpmsg_syntax, helpmsg_example);
	}
	proc_setflag_cmd(dbusrinfo.Notify, helpmsg, Params, RecvChat->Id, dbusrinfo, USRSTATE_SETNOTIFY_INPUT_UID);
}

void fm_processing(DB_User& RecvUserInfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, TGBOT_Message *message)
{
	SMAnsiString &Message = message->Text;
	uint64_t &MessageID = message->Message_Id;

	// обработка нажатия ReplyBtn
	for (auto &cmd : CmdDef)
	{
		if (Message == cmd.replybtn)
			return run_cmd(cmd, RecvUserInfo, nullptr, message);	
	}

	// выполнение команды из интерактивного режима
	auto RUN_CMD = [&RecvUserInfo, &RecvUser, &RecvChat, &MessageID](cmd_proc_func proc, const StringList *params)
	{
		RecvUserInfo.State = USRSTATE_FREE;
		RecvUserInfo.StateParams = "";
		proc(RecvUserInfo, RecvUser, RecvChat, params, MessageID);
	};

	switch (RecvUserInfo.State)
	{
		case USRSTATE_FREE:
			tgbot_SendMessage(RecvChat->Id, BOTMSG_HELPMSG_FIND);
			break;
		case USRSTATE_FIND:
			tgbot_SendMessage(RecvChat->Id, BOTMSG_HELPMSG_CANCEL);
			break;
		case USRSTATE_CHAT:
			{
				GET_USRSTATE_PARAMS(RecvUserInfo.StateParams, USRSTATE_CHAT_params);
				uint64_t recepient = gupvalues.at(0);
				if(recepient) tgbot_SendMessage(recepient, Message);
			}
			break;
		case USRSTATE_SETPERMISSION_INPUT_UID:
		case USRSTATE_SETNOTIFY_INPUT_UID:
			{
				static std::map<int, int> toStateMap = 
				{
					{ USRSTATE_SETPERMISSION_INPUT_UID, USRSTATE_SETPERMISSION_INPUT_NBIT },
					{ USRSTATE_SETNOTIFY_INPUT_UID,     USRSTATE_SETNOTIFY_INPUT_NBIT     },
				};

				// пользователь должен ввести uid или username
				std::unique_ptr<DB_User> usr((Message.IsValidNumber()) ? GetUserByUID(Message) : GetUserByUsername(Message));
				if (!usr)
				{
					RecvUserInfo.State = USRSTATE_FREE;
					RecvUserInfo.StateParams = "";
					SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_USER_NOT_FOUND);
				}
				else
				{
					USRSTATE_INIT_PARAMS(USRSTATE_SETPERMISSION_INPUT_NBIT_params);
					{
						pvalues.push_back(Message);
					}
					RecvUserInfo.State = toStateMap.at(RecvUserInfo.State); //USRSTATE_SETPERMISSION_INPUT_NBIT;
					RecvUserInfo.StateParams = ParamsToJSON(pnames, pvalues);
					tgbot_SendMessage(RecvChat->Id, BOTMSG_INPUT_NBIT);
				}
			}
			break;
		case USRSTATE_SETPERMISSION_INPUT_NBIT:
		case USRSTATE_SETNOTIFY_INPUT_NBIT:
			{
				static std::map<int, int> toStateMap = 
				{
					{ USRSTATE_SETPERMISSION_INPUT_NBIT, USRSTATE_SETPERMISSION_INPUT_VALUE },
					{ USRSTATE_SETNOTIFY_INPUT_NBIT,     USRSTATE_SETNOTIFY_INPUT_VALUE     },
				};

				const unsigned int nbit = Message;
				if (Message.IsValidNumber() && ((nbit>=1) && (nbit<=64)))
				{
					USRSTATE_INIT_PARAMS(USRSTATE_SETPERMISSION_INPUT_VALUE_params);
					{
						GET_USRSTATE_PARAMS(RecvUserInfo.StateParams, USRSTATE_SETPERMISSION_INPUT_NBIT_params);
						pvalues.push_back(gupvalues.at(0)); // uid
						pvalues.push_back(Message);         // nbit
					}
					RecvUserInfo.State = toStateMap.at(RecvUserInfo.State); //USRSTATE_SETPERMISSION_INPUT_VALUE;
					RecvUserInfo.StateParams = ParamsToJSON(pnames, pvalues);
					tgbot_SendMessage(RecvChat->Id, BOTMSG_INPUT_VALUE);
				}
				else
					SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_INVALID_INPUT_NBIT);
			}
			break;
		case USRSTATE_SETPERMISSION_INPUT_VALUE:
		case USRSTATE_SETNOTIFY_INPUT_VALUE:
			{
				static std::map<int, cmd_proc_func> toStateMap =
				{
					{ USRSTATE_SETPERMISSION_INPUT_VALUE, sc_processing_setpermissions },
					{ USRSTATE_SETNOTIFY_INPUT_VALUE,     sc_processing_setnotify      },
				};

				const unsigned int value = Message;
				if (Message.IsValidNumber() && ((value >= 0) && (value <= 1)))
				{
					GET_USRSTATE_PARAMS(RecvUserInfo.StateParams, USRSTATE_SETPERMISSION_INPUT_VALUE_params);
					gupvalues.push_back(Message);
					RUN_CMD(toStateMap.at(RecvUserInfo.State), &gupvalues);
				}
				else
					SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_INVALID_INPUT_VALUE);
			}
			break;
		case USRSTATE_GETPERMISSION_INPUT_UID:
			{
				StringList params = { Message };
				RUN_CMD(sc_processing_getpermissions, &params);
			}
			break;
		case USRSTATE_GETUSERINFO_INPUT_UID:
			{
				StringList params = { Message };
				RUN_CMD(sc_processing_getuserinfo, &params);
			}
			break;
		case USRSTATE_GETNOTIFY_INPUT_UID:
			{
				StringList params = { Message };
				RUN_CMD(sc_processing_getnotify, &params);
			}
			break;
	}
}