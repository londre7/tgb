#ifndef TGBOT_USER_HEADER
#define TGBOT_USER_HEADER

#include "tg_bot.h"

#define PERMISSION_DENIED(chat_id) {tgbot_SendMessage(chat_id,BOTMSG_PERMISSION_DENIED);return;}
#define DEFAULT_USER_PERMISIONS           0x0000000000000000ULL
#define PERMISSION_MANAGE_USR_ACCESS      0x0000000000000001ULL // управление разрешениями пользователей
#define PERMISSION_USR_INFO               0x0000000000000002ULL // просмотр информации о пользователях
#define PERMISSION_FLAG_02                0x0000000000000004ULL
#define PERMISSION_FLAG_03                0x0000000000000008ULL
#define PERMISSION_FLAG_04                0x0000000000000010ULL
#define PERMISSION_FLAG_05                0x0000000000000020ULL
#define PERMISSION_FLAG_06                0x0000000000000040ULL
#define PERMISSION_FLAG_07                0x0000000000000080ULL
#define PERMISSION_FLAG_08                0x0000000000000100ULL
#define PERMISSION_FLAG_09                0x0000000000000200ULL
#define PERMISSION_FLAG_10                0x0000000000000400ULL
#define PERMISSION_FLAG_11                0x0000000000000800ULL
#define PERMISSION_FLAG_12                0x0000000000001000ULL
#define PERMISSION_FLAG_13                0x0000000000002000ULL
#define PERMISSION_FLAG_14                0x0000000000004000ULL
#define PERMISSION_FLAG_15                0x0000000000008000ULL

// уведомления
#define NOTIFY_REGISTER_NEW_USER          0x0000000000000001ULL // регистрация новых пользователей

extern StringList* GetUsrAccessStrList();
extern StringList* GetUsrNotifyStrList();

enum // состояния пользователей
{
	USRSTATE_FREE=0,
	USRSTATE_FIND,
	USRSTATE_CHAT,
	USRSTATE_SETPERMISSION_INPUT_UID,
	USRSTATE_SETPERMISSION_INPUT_NBIT,
	USRSTATE_SETPERMISSION_INPUT_VALUE,
	USRSTATE_GETPERMISSION_INPUT_UID,
	USRSTATE_GETUSERINFO_INPUT_UID,
	USRSTATE_GETNOTIFY_INPUT_UID,
	USRSTATE_SETNOTIFY_INPUT_UID,
	USRSTATE_SETNOTIFY_INPUT_NBIT,
	USRSTATE_SETNOTIFY_INPUT_VALUE
};

// тут объявляем списки параметров для тех состояний, где они нужны (реализация в user.cpp)
#define GET_USRSTATE_PARAMS(json_str, params_def) StringList gupvalues; ParamsFromJSON(json_str, params_def, gupvalues)
#define USRSTATE_INIT_PARAMS(params_def) StringList &pnames = params_def; StringList pvalues
extern StringList  USRSTATE_CHAT_params;
extern StringList  USRSTATE_SETPERMISSION_INPUT_NBIT_params;
extern StringList  USRSTATE_SETPERMISSION_INPUT_VALUE_params;

class DB_User
{
	public:
		uint64_t     UID;
		SMAnsiString FirstName,
		             LastName,
		             Username,
		             LanguageCode;
		time_t       LastMessage;  // дата и вермя с последнего сообщения
		SMAnsiString PhoneNumber;
		uint64_t     Permissions;
		uint64_t     Notify;
		int          State;
		SMAnsiString StateParams;
		
		DB_User(): UID(0ULL), LastMessage(0), State(0), Permissions(0ull), Notify(0ull), StateParams("{}") {}
		DB_User
		(
			uint64_t uid,
			const SMAnsiString &username, 
			const SMAnsiString &firstname, 
			const SMAnsiString &lastname, 
			const SMAnsiString &language_code,
			time_t last_message,
			const SMAnsiString& phone_number,
			uint64_t permissions,
			uint64_t notify,
			int	state,
			SMAnsiString stateParams
		):	
		UID(uid),
		Username(username), 
		FirstName(firstname), 
		LastName(lastname), 
		LanguageCode(language_code), 
		LastMessage(last_message),
		PhoneNumber(phone_number),
		Permissions(permissions),
		Notify(notify),
		State(state),
		StateParams(stateParams)
		{}
		
		DB_User(const DB_User&Itm);

		SMAnsiString InfoStr() const;
		
		DB_User& operator=(const DB_User &Itm);
		bool operator==(const DB_User &Itm);
};

bool IsSuperUser(const SMAnsiString& username);
bool SetUserState(uint64_t uid, int state, const SMAnsiString &json_str);
bool SetUserState(DB_User& usr, int state, const SMAnsiString &json_str);
bool SetUserPhoneNumber(uint64_t uid, const SMAnsiString &phone_number);
bool SetUserPermission(uint64_t uid, uint64_t permission);
bool SetUserNotify(uint64_t uid, uint64_t notify);
SMKeyList* GetUsersListWithNotifyMask(uint64_t notify_mask);
DB_User* GetUserByUID(uint64_t uid);
DB_User* GetUserByUsername(const SMAnsiString& username);

#endif