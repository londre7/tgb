#include "smmysql.h"

bool sm_mysql_query(MYSQL* Connection, SMAnsiString Query)
{
	if (mysql_query(Connection, C_STR(Query)))
		return false;
	if (mysql_field_count(Connection) == 0)
		return false;

	return true;
}

MYSQL_RES* sm_mysql_query_v2(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString & Query)
{
	MYSQL* conn;
	MYSQL_RES* ret;

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, C_STR(Host), C_STR(DBUser), C_STR(DBPassword), C_STR(DBName), 0, NULL, 0))
		return NULL;

	if (mysql_query(conn, C_STR(Query)))
		return NULL;
	if (mysql_field_count(conn) == 0)
		return NULL;

	ret = mysql_store_result(conn);

	mysql_close(conn);
	return ret;
}

SMMYSQL_Table* sm_mysql_query_v3(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString & Query)
{
	MYSQL_RES* res;
	MYSQL_ROW			row;
	SMMYSQL_Table* table = nullptr;

	res = sm_mysql_query_v2(Host, DBUser, DBPassword, DBName, Query); // Host, DBUser, DBPassword, DBName, Query
	if (res == NULL) return NULL;

	if (mysql_num_rows(res) > 0)
	{
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
	{
		table = new SMMYSQL_Table;
	}

	mysql_free_result(res);
	return table;
}

bool sm_mysql_query_insert(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString & Query)
{
	MYSQL* conn;
	MYSQL_RES* ret;

	conn = mysql_init(NULL);
	if (mysql_real_connect(conn, C_STR(Host), C_STR(DBUser), C_STR(DBPassword), C_STR(DBName), 0, NULL, 0) == NULL)
		return false;

	if (mysql_query(conn, C_STR(Query)))
	{
		mysql_close(conn);
		return false;
	}

	ret = mysql_store_result(conn);
	if (ret == nullptr)
	{
		unsigned int err = mysql_errno(conn);
		mysql_close(conn);
		if (err != 0)
			return false;
		else
			return true;
	}
	else
	{
		mysql_free_result(ret);
		mysql_close(conn);
		return true;
	}
}

uint64_t sm_mysql_query_insert_ret_id(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString& Query)
{
	MYSQL* conn;
	MYSQL_RES* ret;
	uint64_t id=0ULL;

	conn = mysql_init(NULL);
	if (mysql_real_connect(conn, Host, DBUser, DBPassword, DBName, 0, NULL, 0) == NULL)
		return false;

	if (mysql_query(conn, Query))
	{
		mysql_close(conn);
		return false;
	}
	ret = mysql_store_result(conn);
	if (ret == nullptr)
	{
		unsigned int err = mysql_errno(conn);
		if (err != 0)
		{
			mysql_close(conn);
			return false;
		}
	}
	else
		mysql_free_result(ret);

	if (mysql_query(conn, "SELECT LAST_INSERT_ID();"))
	{
		mysql_close(conn);
		return false;
	}
	ret = mysql_store_result(conn);
	if (ret == nullptr)
	{
		mysql_close(conn);
		return false;
	}
	if (mysql_num_rows(ret) > 0)
	{
		MYSQL_ROW row;
		std::unique_ptr<SMMYSQL_Table> table(new SMMYSQL_Table(mysql_num_rows(ret), mysql_num_fields(ret)));
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
	mysql_close(conn);

	return id;
}