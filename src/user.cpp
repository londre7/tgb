#include "tg_bot.h"

// тут объявляем параметры для тех состояний, где они нужны
StringList USRSTATE_CHAT_params =
{
	"chatid",
	"innerid"
};
StringList USRSTATE_SETPERMISSION_INPUT_NBIT_params =
{
	"uid"
};
StringList USRSTATE_SETPERMISSION_INPUT_VALUE_params =
{
	"uid",
	"nbit"
};

// права пользователей
static StringList usrAccessStrList =
{
	"Управление правами пользователей",   // PERMISSION_MANAGE_USR_ACCESS
	"Просмотр информации о пользователе", // PERMISSION_USR_INFO
};
// уведомления
static StringList usrNotifyStrList =
{
	"Получение уведомлений",    // NOTIFY_DUMMY
};
StringList* GetUsrAccessStrList() { return &usrAccessStrList; }
StringList* GetUsrNotifyStrList() { return &usrNotifyStrList; }

DB_User::DB_User(const DB_User &Itm)
{
	this->UID = Itm.UID;
	this->FirstName = Itm.FirstName;
	this->LastName = Itm.LastName;
	this->Username = Itm.Username;
	this->LanguageCode = Itm.LanguageCode;
	this->LastMessage = Itm.LastMessage;
	this->PhoneNumber = Itm.PhoneNumber;
	this->Permissions = Itm.Permissions;
	this->Notify = Itm.Notify;
	this->State = Itm.State;
	this->StateParams = Itm.StateParams;
}

SMAnsiString DB_User::InfoStr() const
{
	return SMAnsiString::smprintf
	(
		"<b>ID:</b> %llu\n"
		"<b>Username:</b> %s\n"
		"<b>Фамилия:</b> %s\n"
		"<b>Имя:</b> %s\n"
		"<b>Язык:</b> %s\n"
		"<b>Номер телефона:</b> %s\n"
		"<b>Права доступа:</b> недосутпно\n"
		"<b>Уведомления:</b> недосутпно\n"
		"<b>Последняя активность:</b> %s",
		this->UID,
		C_STR(MakeUsername(this->Username)),
		C_STR(this->LastName),
		C_STR(this->FirstName),
		C_STR(this->LanguageCode),
		C_STR(this->PhoneNumber),
		C_STR(SMDateTime(this->LastMessage).DateTimeString())
	);
}

DB_User& DB_User::operator=(const DB_User &Itm)
{
	this->UID = Itm.UID;
	this->FirstName = Itm.FirstName;
	this->LastName = Itm.LastName;
	this->Username = Itm.Username;
	this->LanguageCode = Itm.LanguageCode;
	this->LastMessage = Itm.LastMessage;
	this->PhoneNumber = Itm.PhoneNumber;
	this->Permissions = Itm.Permissions;
	this->Notify = Itm.Notify;
	this->State = Itm.State;
	this->StateParams = Itm.StateParams;
	
	return *this;
}

bool DB_User::operator==(const DB_User&Itm)
{
	return	(this->UID == Itm.UID) && 
			(this->FirstName == Itm.FirstName) && 
			(this->LastName == Itm.LastName) && 
			(this->Username == Itm.Username) && 
			(this->LanguageCode == Itm.LanguageCode);
}

bool IsSuperUser(const SMAnsiString& username)
{
	return (username == GetBotConf()->GetParam(BotConfStruct::SuperUser));
}

bool SetUserState(uint64_t uid, int state, const SMAnsiString &json_str)
{
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_users SET state=%d, state_params=\'%s\' WHERE uid=%llu;",
			state,
			C_STR(json_str),
			uid
		)
	);
}

bool SetUserState(const SMKeyList& list, int state, const SMAnsiString& json_str)
{
	if (!list.GetNumElem()) return false;
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_users SET state=%d, state_params=\'%s\' WHERE %s;",
			state,
			C_STR(json_str),
			C_STR(MakeKeyListStr(list))
		)
	);
}

bool SetUserState(DB_User& usr, int state, const SMAnsiString& json_str)
{
	usr.State = state;
	usr.StateParams = json_str;
	return true;
}

bool SetUserPhoneNumber(uint64_t uid, const SMAnsiString &phone_number)
{
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_users SET phone_number=\'%s\' WHERE uid=%llu;",
			C_STR(phone_number),
			uid
		)
	);
}

bool SetUserPermission(uint64_t uid, uint64_t permission)
{
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_users SET access=%llu WHERE uid=%llu;",
			permission,
			uid
		)
	);
}

bool SetUserNotify(uint64_t uid, uint64_t notify)
{
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_users SET notify=%llu WHERE uid=%llu;",
			notify,
			uid
		)
	);
}

SMKeyList* GetUsersListWithNotifyMask(uint64_t notify_mask)
{
	MySQLTablePtr usr_tbl
	(
		QueryFromDB
		(
			SMAnsiString::smprintf
			(
				"SELECT uid FROM tgb_users WHERE notify&%llu;",
				notify_mask
			)
		)
	);

	if (!usr_tbl) 
		return nullptr;

	SMKeyList *ret = new SMKeyList;
	const size_t rows = usr_tbl->Rows;
	for (size_t i = 0; i < rows; i++)
	{
		ret->push_back(usr_tbl->Cell[i][0]);
	}
	return ret;
}

static DB_User* GetUserFromDB(const SMAnsiString& condition)
{
	MySQLTablePtr user_tbl
	(
		QueryFromDB
		(
			SMAnsiString::smprintf
			(
				"SELECT * FROM tgb_users WHERE %s",
				C_STR(condition)
			)
		)
	);
	if (!user_tbl) return nullptr;
	if (!user_tbl->Rows) return nullptr;

	uint64_t uid = user_tbl->Cell[0][1];
	SMAnsiString &username = user_tbl->Cell[0][2];
	SMAnsiString &firstname = user_tbl->Cell[0][3];
	SMAnsiString &lastname = user_tbl->Cell[0][4];
	SMAnsiString &language_code = user_tbl->Cell[0][5];
	time_t last_message_time = user_tbl->Cell[0][6];
	SMAnsiString &phone_number = user_tbl->Cell[0][7];
	uint64_t permissions = user_tbl->Cell[0][8];
	uint64_t notify = user_tbl->Cell[0][9];
	int state = user_tbl->Cell[0][10];
	SMAnsiString &state_params = user_tbl->Cell[0][11];

	return new DB_User(uid, username, firstname, lastname, language_code, last_message_time, phone_number, permissions, notify, state, state_params);
}

DB_User* GetUserByUID(uint64_t uid)
{
	return GetUserFromDB(SMAnsiString::smprintf("uid=%llu", uid));
}

DB_User* GetUserByUsername(const SMAnsiString &username)
{
	return GetUserFromDB(SMAnsiString::smprintf("username=\'%s\'", C_STR(username)));
}