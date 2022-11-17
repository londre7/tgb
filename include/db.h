#ifndef TGBOT_DB_HEADER
#define TGBOT_DB_HEADER

#include "tg_bot.h"

typedef std::unique_ptr<SMMYSQL_Table> MySQLTablePtr;

extern bool InsertToDB(const SMAnsiString& query);
extern uint64_t InsertToDBWithRetID(const SMAnsiString& query);
extern SMMYSQL_Table* QueryFromDB(const SMAnsiString& query);
extern std::vector<std::unique_ptr<SMMYSQL_Table>> QueryFromDB(const StringList& queryList);

#endif // TGBOT_DB_HEADER