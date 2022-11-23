// здесь описываем обработчики для команд "/"
#include "tg_bot.h"

// список команд и их обработчики
typedef void (*cmd_proc_func)(DB_User&, TGBOT_User*, TGBOT_Chat*, const StringList*, uint64_t);
struct Cmd { const char* cmd; const char* replybtn; cmd_proc_func proc; int usrstate; uint64_t permission; bool and_flag; } 
CmdDef[] =
{
	// команда           соотв. кнопка            обработчик                    необх. сост.   необходимые разрешения        флаг "И"  
	{ "/start",          nullptr,                 sc_processing_start,          USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/cmdlist",        nullptr,                 sc_processing_cmdlist,        USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/find",           REPLYBTN_CAPTION_FIND,   sc_processing_find,           USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/cancel",         REPLYBTN_CAPTION_CANCEL, sc_processing_cancel,         USRSTATE_ALL,  DEFAULT_USER_PERMISIONS,      false },
	{ "/stop",           REPLYBTN_CAPTION_STOP,   sc_processing_stop,           USRSTATE_CHAT, DEFAULT_USER_PERMISIONS,      false },
	{ "/ap",             nullptr,                 sc_processing_ap,             USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/getpermissions", nullptr,                 sc_processing_getpermissions, USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/setpermissions", nullptr,                 sc_processing_setpermissions, USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/getuserinfo",    nullptr,                 sc_processing_getuserinfo,    USRSTATE_FREE, FLAGS_0_1,                    false },
	{ "/getnotify",      nullptr,                 sc_processing_getnotify,      USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/setnotify",      nullptr,                 sc_processing_setnotify,      USRSTATE_FREE, PERMISSION_MANAGE_USR_ACCESS, false },
	{ "/id",             nullptr,                 sc_processing_id,             USRSTATE_FREE, DEFAULT_USER_PERMISIONS,      false },
	{ "/sendmsg",        nullptr,                 sc_processing_sendmsg,        USRSTATE_FREE, PERMISSION_SENDMSG,           false },
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
	static const std::map<int, const char*> msgMap =
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

static void run_cmd(const Cmd &cmd, DB_User &dbusrinfo, const SMAnsiString &param, TGBOT_Message* message)
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
	cmd.proc(dbusrinfo, message->From, message->Chat, params.get(), message->MessageId);
	return;
}

void RunProcCmd(const SMAnsiString &cmd, const SMAnsiString &param, TGBOT_Message* message, DB_User& dbusrinfo)
{
	// выполняем команду
	for (auto& command : CmdDef)
	{
		if (cmd == command.cmd)
			return run_cmd(command, dbusrinfo, param, message);
	}
	// неизвестная команда
	sc_processing_unknown(dbusrinfo, message->From, message->Chat, param, message->MessageId);
}

void sc_processing_unknown(DB_User& dbusrinfo, TGBOT_User *RecvUser, TGBOT_Chat *RecvChat, const SMAnsiString &Params, uint64_t MessageID)
{
	tgbot_SendMessage(RecvChat->Id, BOTMSG_CMD_UNKNOWN);
}

void sc_processing_start(DB_User& dbusrinfo, TGBOT_User *RecvUser, TGBOT_Chat *RecvChat, const StringList* Params, uint64_t MessageID)
{
	// меню
	static const std::vector<InlineKeyboardDef> kb_start_decl =
	{
		{ INLINEBTN_CAPTION_PRIVATE_POLICY, CALLBACK_PRIVATE_POLICY, nullptr },
		{ "#newrow",                        nullptr,                 nullptr },
		{ INLINEBTN_CAPTION_CMDLIST,        CALLBACK_CMDLIST,        nullptr },
		{ "#newrow",                        nullptr,                 nullptr },
		{ INLINEBTN_CAPTION_ID,             CALLBACK_ID,             nullptr },
		{ "#newrow",                        nullptr,                 nullptr },
		{ INLINEBTN_CAPTION_FIND,           CALLBACK_FIND,           nullptr },
	};

	#if 0
	// параметры для callback
	StringList valuesAbout = { "param" };
	std::unique_ptr<TGBOT_InlineKeyboardMarkup> kb_cmd(MakeInlineKeyboardFromDef(kb_start, { nullptr,&valuesAbout }));
	#endif

	std::unique_ptr<TGBOT_InlineKeyboardMarkup> kb_start(MakeInlineKeyboardFromDef(kb_start_decl, { nullptr,nullptr,nullptr,nullptr,nullptr }));

	// посылаем сообщенние с клавой
	SMAnsiString text = SMAnsiString::smprintf(BOTMSG_CMD_START, C_STR(MakeFullUserName(RecvUser)), STR_DONATIONS_REQUSITS);
	tgbot_SendMessage(RecvChat->Id, text, kb_start.get());
}

void sc_processing_cmdlist(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SMAnsiString helptext = "📜 <b>Список команд</b>:\n"
	                        "<b>/cmdlist</b> - вывести список доступных команд;\n"
	                        "<b>/find</b> - запуск поиска собеседника\n"
		                    "<b>/cancel</b> - отменить поиск\n"
	                        "<b>/stop</b> - завершить беседу";
	SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, helptext, REPLYBTN_CAPTION_FIND);
}

void sc_processing_find(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
#if 0
	// проверяем, может ли пользователь пользоваться чатом
	const size_t MAX_FREE_CHATS_PER_DAY = 5ull;
	const time_t dayBegin = GetBeginDay(time(NULL));
	size_t n = GetNumChatsForUser(RecvUser->Id, dayBegin, dayBegin+86400);
	if (n >= MAX_FREE_CHATS_PER_DAY)
		SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_LIMIT_FREE_CHATS);
#endif // 0

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
		case USRSTATE_SENDMSG_INPUT_UID:
		case USRSTATE_SENDMSG_INPUT_MSG:
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
	uint64_t innerID = gupvalues.at(1);

	// закрываем чат
	if(!StopChat(innerID, RecvUser->Id))
		SEND_MSG_AND_RETURN(RecvChat->Id, BOTMSG_INTERNAL_ERROR);

	// переводим пользователей в обычное состояние
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
	const size_t numparam = Params ? Params->size() : 0ull;
	if (!numparam)
	{
		// переводим в интерактивный режим
		dbusrinfo.State = to_state;
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
	}
	if (numparam != 1)
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	const SMAnsiString &target_user = Params->at(0);
	std::unique_ptr<DB_User> usr((target_user.IsValidNumber()) ? GetUserByUID(target_user) : GetUserByUsername(target_user));
	if (!usr)
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_USER_NOT_FOUND, REPLYBTN_CAPTION_FIND);

	SEND_MSG_AND_RETURN_WITH_BTN(chatId, MakePermissionString(flag, str), REPLYBTN_CAPTION_FIND);
}

static void proc_setflag_cmd(uint64_t &flag, const SMAnsiString& helpmsg, const StringList* Params,
	uint64_t chatId, DB_User& dbusrinfo, int to_state)
{
	// проверяем количество параметров
	const size_t numparam = Params ? Params->size() : 0ull;
	if (!numparam)
	{
		// переводим в интерактивный режим
		dbusrinfo.State = to_state;
		SEND_MSG_AND_RETURN_WITH_BTN(chatId, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
	}
	if (numparam != 3)
		SEND_MSG_AND_RETURN(chatId, helpmsg);

	// достаём параметры
	const SMAnsiString &target_user = Params->at(0);
	const SMAnsiString &nbit_str = Params->at(1);
	const SMAnsiString &setval_str = Params->at(2);
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
	const size_t numparam = Params ? Params->size() : 0ull;
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

void sc_processing_id(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, SMAnsiString::smprintf(BOTMSG_USER_ID, dbusrinfo.UID), REPLYBTN_CAPTION_FIND);
}

void sc_processing_sendmsg(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID)
{
	SetUserState(dbusrinfo, USRSTATE_SENDMSG_INPUT_UID);
	SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, BOTMSG_INPUT_UID, REPLYBTN_CAPTION_CANCEL);
}

void fm_processing(DB_User& RecvUserInfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, TGBOT_Message *message)
{
	const SMAnsiString &Message = message->Text;
	uint64_t &MessageID = message->MessageId;

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
				static const std::map<int, int> toStateMap = 
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
				static const std::map<int, int> toStateMap = 
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
						pvalues.push_back(std::move(gupvalues.at(0))); // uid
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
				static const std::map<int, cmd_proc_func> toStateMap =
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
		case USRSTATE_GETUSERINFO_INPUT_UID:
		case USRSTATE_GETNOTIFY_INPUT_UID:
			{
				static const std::map<int, cmd_proc_func> cmdMap =
				{
					{ USRSTATE_GETPERMISSION_INPUT_UID, sc_processing_getpermissions },
					{ USRSTATE_GETUSERINFO_INPUT_UID,   sc_processing_getuserinfo    },
					{ USRSTATE_GETNOTIFY_INPUT_UID,     sc_processing_getnotify      },
				};
				StringList params = { Message };
				RUN_CMD(cmdMap.at(RecvUserInfo.State), &params);
			}
			break;
		case USRSTATE_SENDMSG_INPUT_UID:
			{
				std::unique_ptr<DB_User> usr((Message.IsValidNumber()) ? GetUserByUID(Message) : GetUserByUsername(Message));
				if (!usr)
				{
					SetUserState(RecvUserInfo, USRSTATE_FREE);
					SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, BOTMSG_USER_NOT_FOUND, REPLYBTN_CAPTION_FIND);
				}
				SetUserState(RecvUserInfo, USRSTATE_SENDMSG_INPUT_MSG, ParamsToJSON(USRSTATE_SENDMSG_INPUT_MSG_params,{usr->UID}));
				tgbot_SendMessage(RecvChat->Id, SMAnsiString::smprintf(BOTMSG_INPUT_MSGTEXT, C_STR(MakeFullUserName(usr.get(),true))));
			}
			break;
		case USRSTATE_SENDMSG_INPUT_MSG:
			{
				GET_USRSTATE_PARAMS(RecvUserInfo.StateParams, USRSTATE_SENDMSG_INPUT_MSG_params);
				uint64_t to_uid = gupvalues.at(0);
				SMAnsiString msg = SMAnsiString::smprintf
				(
					BOTMSG_FROM_ADMIN,
					C_STR(MakeFullUserName(&RecvUserInfo, true)),
					C_STR(Message)
				);
				tgbot_SendMessage(to_uid, msg);
				SetUserState(RecvUserInfo, USRSTATE_FREE);
				SEND_MSG_AND_RETURN_WITH_BTN(RecvChat->Id, BOTMSG_SUCCESSFULL, REPLYBTN_CAPTION_FIND);
			}
			break;
	}
}