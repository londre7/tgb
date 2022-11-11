#ifndef TGBOT_DB_HEADER
#define TGBOT_DB_HEADER

#include "tg_bot.h"

typedef std::unique_ptr<SMMYSQL_Table> MySQLTablePtr;

extern bool InsertToDB(const SMAnsiString& query);
extern SMMYSQL_Table* QueryFromDB(const SMAnsiString& query);

#endif // TGBOT_DB_HEADER