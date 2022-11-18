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

uint64_t InsertToDBWithRetID(const SMAnsiString& query)
{
	BotConfStruct* bot_conf = GetBotConf();
	return sm_mysql_query_insert_ret_id
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

std::vector<std::unique_ptr<SMMYSQL_Table>> QueryFromDB(const StringList& queryList)
{
	BotConfStruct* bot_conf = GetBotConf();
	return sm_mysql_query_list
	(
			bot_conf->GetParam(BotConfStruct::DBHost),
			bot_conf->GetParam(BotConfStruct::DBUser),
			bot_conf->GetParam(BotConfStruct::DBPassword),
			bot_conf->GetParam(BotConfStruct::DBDatabase),
			queryList
	);
}