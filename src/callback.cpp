#include "tg_bot.h"

// список колбэков и их обработчики
struct
{
	const char *token;
	void (*proc)(DB_User&, TGBOT_User*, TGBOT_Message*, const SMAnsiString&, const StringList&);
	const StringList *paramDef;
} 
CallbackDef[] =
{
	// callback_data                  обработчик                           параметры для обработки callback    
	{ CALLBACK_PRIVATE_POLICY,        cq_processing_private_policy,        nullptr                           },
	{ CALLBACK_CMDLIST,               cq_processing_cmdlist,               nullptr                           },
	{ CALLBACK_FIND,                  cq_processing_find,                  nullptr                           },
	{ CALLBACK_ID,                    cq_processing_id,                    nullptr                           },
	{ CALLBACK_REPLY_TO_SENDMSG,      cq_processing_reply_to_sendmsg,      &CallbackParamsDef_ReplyToSendMsg },
};

// параметры для callback'ов
StringList CallbackParamsDef_ReplyToSendMsg =
{
	"admin_uid" // UID администратора
};

static SMAnsiString GenerateCallbackID()
{
	static char   prev_callback_hash[33];
	char          callback_hash[33];
	unsigned char callback_hash_bin[32];

	// гегенируем callback
	memset(callback_hash, 0, 32);
	uint64_t r1 = 0ULL + rand() % 18446744073709551615ULL;
	uint64_t r2 = 0ULL + rand() % 18446744073709551615ULL;
	unsigned char r[16];
	memcpy(&r[0], &r1, sizeof(r1));
	memcpy(&r[8], &r2, sizeof(r2));
	SHA256(r, sizeof(r), callback_hash_bin);
	Hexlify(callback_hash, callback_hash_bin, 16);
	return SMAnsiString(callback_hash);
}

// для каждой callback кнопки задаём список параметров, который представлен StringList, количество таких StringList == количество кнопок
std::vector<std::unique_ptr<StringList>> MakeCallbackParamValues(const std::vector<InlineKeyboardDef>& KbDecl, int dummy, ...)
{
	std::vector<std::unique_ptr<StringList>> ret;

	va_list variadic_p;
	va_start(variadic_p, dummy);
	const size_t numbtns = KbDecl.size();
	for (size_t i = 0; i < numbtns; i++)
	{
		std::initializer_list<SMAnsiString> p_arg = va_arg(variadic_p, std::initializer_list<SMAnsiString>);
		std::unique_ptr<StringList> uptr((p_arg.size())?new StringList(p_arg):nullptr);
		ret.push_back(std::move(uptr));
	}
	va_end(variadic_p);

	return std::move(ret);
}

// для одной кнопки
SMAnsiString MakeCallbackData(const SMAnsiString &type, StringList *keys, StringList *values)
{
	SMAnsiString id = GenerateCallbackID();
	if (keys && values)
	{
		InsertToDB
		(
			SMAnsiString::smprintf
			(
				"INSERT INTO tgb_callbacks (callback_id, callback_type, callback_params) VALUES (\'%s\', \'%s\', \'%s\');",
				C_STR(id),
				C_STR(type),
				C_STR(ParamsToJSON(*keys, *values))
			)
		);
	}	
	return SMAnsiString::smprintf("%s_%s", C_STR(id), C_STR(type));
}
// для пачки
bool RegisterCallbackData(const StringList &callback, const std::vector<StringList*>& params, const std::vector<std::unique_ptr<StringList>> &values)
{
	auto MAKE_SQL_VALUE = [](const SMAnsiString & callback, const StringList * keys, const StringList * values)->SMAnsiString
	{
		const int pos = callback.Pos('_');
		if (pos == -1) return "";

		SMAnsiString refref = callback;
		refref[pos] = '\0';
		return SMAnsiString::smprintf("(\'%s\',\'%s\',\'%s\')", C_STR(refref), &refref[pos + 1], C_STR(ParamsToJSON(*keys, *values)));
	};

	const size_t callbacknum = callback.size();
	const size_t paramsnum = params.size();
	const size_t valuesnum = values.size();
	if ((callbacknum != paramsnum) || (callbacknum != valuesnum))
		return false;

	SMAnsiString querystr("INSERT INTO tgb_callbacks (callback_id, callback_type, callback_params) VALUES ");
	size_t values_cnt=0ull;
	for (size_t i = 0; i < callbacknum; i++)
	{
		const StringList* paramslist = params.at(i);
		if (!paramslist) 
			continue;
		if (values_cnt++) querystr += ",";
		querystr += MAKE_SQL_VALUE(callback.at(i), paramslist, values.at(i).get());
	}

	return (values_cnt)?InsertToDB(querystr):true;
}

static SMAnsiString GetPressedBtnCaption(const TGBOT_CallbackQuery* RecvCallback)
{
	const TGBOT_InlineKeyboardMarkup* kb = RecvCallback->Message->ReplyMarkup;
	const size_t numrow = kb->InlineKeyboard.size();
	for (size_t i = 0; i < numrow; i++)
	{
		const size_t numbtn = kb->InlineKeyboard.at(i).size();
		for (size_t j = 0; j < numbtn; j++)
		{
			if (kb->InlineKeyboard[i][j]->CallbackData == RecvCallback->Data)
				return kb->InlineKeyboard[i][j]->Text;
		}
	}
	return SMAnsiString();
}

void RunCallbackProc(TGBOT_CallbackQuery* RecvCallback, DB_User &dbusrinfo)
{
	WriteFormatMessage(SYSTEMMSG_RECV_CALLBACK_QUERY, TGB_TEXTCOLOR_YELLOW, C_STR(RecvCallback->Data), C_STR(RecvCallback->From->Username), C_STR(RecvCallback->From->FirstName));

	const SMAnsiString &callback_token = RecvCallback->Data;
	const SMAnsiString callback_id = callback_token.Delete(32ull, callback_token.length());
	const SMAnsiString callback_type = callback_token.Delete(0ull, 33ull);
	SMAnsiString callback_params = "{}";

	bool is_valid_callbacktype = false;
	for(auto &callback: CallbackDef)
	{
		if (callback_type == callback.token)
		{
			// лезем в БД за параметрами callback
			MySQLTablePtr CallbackTbl
			(
				QueryFromDB
				(
					SMAnsiString::smprintf("SELECT * FROM tgb_callbacks WHERE callback_id=\'%s\' AND callback_type=\'%s\';", C_STR(callback_id), C_STR(callback_type))
				)
			);
			if (CallbackTbl && CallbackTbl->Rows > 1)
				WriteMessage(SMAnsiString::smprintf(SYSTEMMSG_DUPLICATE_CALLBACK, C_STR(callback_id), C_STR(callback_type)), TGB_TEXTCOLOR_YELLOW);

			bool valid_callback = true;
			uint64_t to_uid = 0ull;
			if (CallbackTbl && (CallbackTbl->Rows == 1))
			{
				callback_params = CallbackTbl->Cell[0][3];
				// проводим валидацию
				to_uid = CallbackTbl->Cell[0][4]; // TODO: в будущем может понадобиться, например - если бот находится в групповом чате
				valid_callback = !(uint64_t(CallbackTbl->Cell[0][5]) > 0);
			}

			if (valid_callback && (!to_uid || (to_uid==dbusrinfo.UID)))
			{
				// готовим параметры
				StringList param_values; 
				if(callback.paramDef)
					ParamsFromJSON(callback_params, *callback.paramDef, param_values);

				// запускаем обработчик
				callback.proc(dbusrinfo, RecvCallback->From, RecvCallback->Message, GetPressedBtnCaption(RecvCallback), param_values);

				// ставим, что обработали
				if (CallbackTbl && (CallbackTbl->Rows == 1))
				{
					InsertToDB
					(
						SMAnsiString::smprintf
						(
							"UPDATE tgb_callbacks SET activate_uid=%llu WHERE callback_id=\'%s\' AND callback_type=\'%s\';",
							RecvCallback->From->Id,
							C_STR(callback_id),
							C_STR(callback_type)
						)
					);
				}
			}
			is_valid_callbacktype = true;
			break;
		}
	}
	if (!is_valid_callbacktype)
		WriteFormatMessage(SYSTEMMSG_UNEXPECTED_CALLBACK, TGB_TEXTCOLOR_RED, C_STR(RecvCallback->Data));

	// отправляем ответ на колбэк
	tgbot_answerCallbackQuery(RecvCallback->Id);
}

// частоповторяющиеся куски кода
#define CHECK_USRSTATE(dbuser, state) if(dbuser.State != state) { return; }
#define CALLBACK_INIT_PARAMS(params_def) StringList *cpnames = params_def; StringList cpvalues
#define CALLBACK_REINIT_PARAMS(params_def) cpnames = params_def; cpvalues.clear()

// Telegram API в качестве сообщения передаёт сообщение, к которому привязана кнопка.
// В связи с этим в обработчике команды в поле From будет передан бот, а не пользователь, который нажал кнопку.
// Собсна этот хак нужен чтоб подменить бота на юзера, который нажал кнопку.
// Т.е. если внутри обработчиков команд sc_processing_* есть обращения к From, то перед вызовом RunProcCmd
// нужно вызвать этот хак
static void DirtyHack(TGBOT_Message* msg, const TGBOT_User* from)
{
	msg->From->Id = from->Id;
	msg->From->Is_Bot = from->Is_Bot;
	msg->From->IsPremium = from->IsPremium;
	msg->From->FirstName = from->FirstName;
	msg->From->LastName = from->LastName;
	msg->From->Username = from->Username;
	msg->From->LanguageCode = from->LanguageCode;
}

void cq_processing_private_policy(CQ_PROCESSING_PARAMS)
{
	CHECK_USRSTATE_FREE(dbusrinfo, Message->Chat->Id);
	tgbot_SendMessage(From->Id, BOTMSG_PRIVATE_POLICY);
}

void cq_processing_cmdlist(CQ_PROCESSING_PARAMS)
{
	RunProcCmd("/cmdlist", "", Message, dbusrinfo);
	// образец как достать параметры
	//const char *chapter = Params.at(0).c_str();
}

void cq_processing_find(CQ_PROCESSING_PARAMS)
{
	RunProcCmd("/find", "", Message, dbusrinfo);
}

void cq_processing_id(CQ_PROCESSING_PARAMS)
{
	RunProcCmd("/id", "", Message, dbusrinfo);
}

void cq_processing_reply_to_sendmsg(CQ_PROCESSING_PARAMS)
{
	CHECK_USRSTATE_FREE(dbusrinfo, Message->Chat->Id);
	const SMAnsiString &admin_uid = Params.at(0);
	DBUserPtr admin(GetUserByUID(static_cast<uint64_t>(admin_uid)));
	if (!admin)
		SEND_MSG_AND_RETURN(dbusrinfo.UID, BOTMSG_INTERNAL_ERROR);

	USRSTATE_INIT_PARAMS(USRSTATE_MESSAGE_REPLY_params);
	pvalues.push_back(admin_uid);
	SetUserState(dbusrinfo, USRSTATE_MESSAGE_REPLY, ParamsToJSON(pnames, pvalues));

	SMAnsiString msg = SMAnsiString::smprintf
	(
		BOTMSG_INPUT_MSGTEXT,
		C_STR(MakeFullUserName(admin.get(), true))
	);
	SEND_MSG_AND_RETURN_WITH_BTN(dbusrinfo.UID, msg, REPLYBTN_CAPTION_CANCEL);
}

#undef CHECK_USRSTATE
#undef CALLBACK_INIT_PARAMS
#undef CALLBACK_REINIT_PARAMS