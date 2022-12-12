#include "smmysql.h"

MYSQL_RES* sm_mysql_query_v2(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query)
{
	MYSQL conn;
	mysql_init(&conn);
	if (!mysql_real_connect(&conn, C_STR(Host), C_STR(DBUser), C_STR(DBPassword), C_STR(DBName), 0, nullptr, 0))
		return nullptr;

	#define CLOSE_CON_AND_RETURN_PTR(conn, ptr) { mysql_close(&conn); return ptr; }
	if (mysql_query(&conn, C_STR(Query)))
		CLOSE_CON_AND_RETURN_PTR(conn, nullptr);
	if (mysql_field_count(&conn) == 0)
		CLOSE_CON_AND_RETURN_PTR(conn, nullptr);

	MYSQL_RES *ret = mysql_store_result(&conn);
	CLOSE_CON_AND_RETURN_PTR(conn, ret);
	#undef CLOSE_CON_AND_RETURN_PTR
}

SMMYSQL_Table* sm_mysql_query_v3(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query)
{
	MYSQL_RES *res = sm_mysql_query_v2(Host, DBUser, DBPassword, DBName, Query); // Host, DBUser, DBPassword, DBName, Query
	if (res == nullptr) return nullptr;

	SMMYSQL_Table *table = nullptr;
	if (mysql_num_rows(res) > 0)
	{
		MYSQL_ROW row;
		table = new SMMYSQL_Table(mysql_num_rows(res), mysql_num_fields(res));
		for (size_t i = 0; row = mysql_fetch_row(res); i++)
		{
			for (size_t j = 0; j < table->Cols; j++)
			{
				if (row[j] != NULL) table->Cell[i][j] = row[j];
				else table->Cell[i][j] = "";
			}
		}
	}
	else
		table = new SMMYSQL_Table;

	mysql_free_result(res);
	return table;
}

std::vector<MySQLTablePtr> sm_mysql_query_list(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const std::vector<SMAnsiString> &queryList)
{
	#define PUSH_BACK_AND_CONTINUE(vec, obj) { vec.push_back(obj); continue; }
	std::vector<MySQLTablePtr> ret;

	const size_t numquery = queryList.size();
	if (!numquery) return std::move(ret);

	MYSQL conn;
	mysql_init(&conn);
	if (!mysql_real_connect(&conn, C_STR(Host), C_STR(DBUser), C_STR(DBPassword), C_STR(DBName), 0, NULL, 0))
		return std::move(ret);
	for (auto &query : queryList)
	{
		if (mysql_query(&conn, C_STR(query)) || (mysql_field_count(&conn) == 0))
			PUSH_BACK_AND_CONTINUE(ret, nullptr);

		SMMYSQL_Table* table = nullptr;
		MYSQL_RES* res = mysql_store_result(&conn);
		if(!res)
			PUSH_BACK_AND_CONTINUE(ret, nullptr);

		if (mysql_num_rows(res) > 0)
		{
			MYSQL_ROW row;
			table = new SMMYSQL_Table(mysql_num_rows(res), mysql_num_fields(res));
			for (size_t i = 0; row = mysql_fetch_row(res); i++)
			{
				for (size_t j = 0; j < table->Cols; j++)
				{
					if (row[j] != NULL) table->Cell[i][j] = row[j];
					else table->Cell[i][j] = "";
				}
			}
		}
		else
			table = new SMMYSQL_Table;

		MySQLTablePtr ptr(table);
		ret.push_back(std::move(ptr));
		mysql_free_result(res);
	}
	mysql_close(&conn);

	return std::move(ret);
	#undef PUSH_BACK_AND_CONTINUE
}

bool sm_mysql_query_insert(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query)
{
	MYSQL conn;
	mysql_init(&conn);
	if (mysql_real_connect(&conn, C_STR(Host), C_STR(DBUser), C_STR(DBPassword), C_STR(DBName), 0, NULL, 0) == NULL)
		return false;

	if (mysql_query(&conn, C_STR(Query)))
	{
		mysql_close(&conn);
		return false;
	}

	MYSQL_RES *ret = mysql_store_result(&conn);
	if (ret == nullptr)
	{
		unsigned int err = mysql_errno(&conn);
		mysql_close(&conn);
		if (err != 0)
			return false;
		else
			return true;
	}
	else
	{
		mysql_free_result(ret);
		mysql_close(&conn);
		return true;
	}
}

uint64_t sm_mysql_query_insert_ret_id(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString& Query)
{
	MYSQL conn;
	mysql_init(&conn);
	if (mysql_real_connect(&conn, Host, DBUser, DBPassword, DBName, 0, NULL, 0) == NULL)
		return 0ull;

	if (mysql_query(&conn, Query))
	{
		mysql_close(&conn);
		return 0ull;
	}
	MYSQL_RES *ret = mysql_store_result(&conn);
	if (ret == nullptr)
	{
		unsigned int err = mysql_errno(&conn);
		if (err != 0)
		{
			mysql_close(&conn);
			return 0ull;
		}
	}
	else
		mysql_free_result(ret);

	if (mysql_query(&conn, "SELECT LAST_INSERT_ID();"))
	{
		mysql_close(&conn);
		return 0ull;
	}
	ret = mysql_store_result(&conn);
	if (ret == nullptr)
	{
		mysql_close(&conn);
		return 0ull;
	}
	uint64_t id = 0ull;
	if (mysql_num_rows(ret) > 0)
	{
		MYSQL_ROW row;
		MySQLTablePtr table(new SMMYSQL_Table(mysql_num_rows(ret), mysql_num_fields(ret)));
		for (size_t i = 0; row = mysql_fetch_row(ret); i++)
		{
			for (size_t j = 0; j < table->Cols; j++)
			{
				if (row[j] != NULL) table->Cell[i][j] = row[j];
				else table->Cell[i][j] = "";
			}
		}
		id = table->Cell[0][0];
	}
	mysql_free_result(ret);
	mysql_close(&conn);

	return id;
}