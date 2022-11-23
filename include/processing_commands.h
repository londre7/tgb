#ifndef TGBOT_PROCESSING_COMMANDS_HEADER
#define TGBOT_PROCESSING_COMMANDS_HEADER

#include "tg_bot.h"

void RunProcCmd(const SMAnsiString &cmd, const SMAnsiString &param, TGBOT_Message* message, DB_User& dbusrinfo);

// обработчики команд
void sc_processing_start(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_cmdlist(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_find(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_cancel(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_stop(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_ap(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_getpermissions(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_setpermissions(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_getuserinfo(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_getnotify(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_setnotify(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_id(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);
void sc_processing_sendmsg(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const StringList* Params, uint64_t MessageID);

void sc_processing_unknown(DB_User& dbusrinfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, const SMAnsiString &Params, uint64_t MessageID);
void fm_processing(DB_User& RecvUserInfo, TGBOT_User* RecvUser, TGBOT_Chat* RecvChat, TGBOT_Message *message);

#endif