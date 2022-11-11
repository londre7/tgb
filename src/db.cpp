#include "tg_bot.h"

bool InsertToDB(const SMAnsiString &query)
{
	BotConfStruct* bot_conf = GetBotConf();
	return sm_mysql_query_insert
	(
		bot_conf->GetParam(BotConfStruct::DBHost),
		bot_conf->GetParam(BotConfStruct::DBUser),
		bot_conf->GetParam(BotConfStruct::DBPassword),
		bot_conf->GetParam(BotConfStruct::DBDatabase),
		query
	);
}

SMMYSQL_Table* QueryFromDB(const SMAnsiString& query)
{
	BotConfStruct* bot_conf = GetBotConf();
	return sm_mysql_query_v3
	(
		bot_conf->GetParam(BotConfStruct::DBHost),
		bot_conf->GetParam(BotConfStruct::DBUser),
		bot_conf->GetParam(BotConfStruct::DBPassword),
		bot_conf->GetParam(BotConfStruct::DBDatabase),
		query
	);
}