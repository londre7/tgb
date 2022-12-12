#ifndef SM_MYSQL_UTILS
#define SM_MYSQL_UTILS

#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include "smstring.h"

#define DELETE_SINGLE_OBJECT(_mptr) if(_mptr != nullptr) { delete _mptr; _mptr = nullptr; }
#define DELETE_ARRAY_OBJECT(_mptr) if(_mptr != nullptr) { delete[] _mptr; _mptr = nullptr; }

class SMMYSQL_Table
{
	public:
		SMAnsiString** Cell = nullptr;	// €чейки таблицы
		size_t         Cols = 0ull;
		size_t         Rows = 0ull;

		SMMYSQL_Table() {}
		SMMYSQL_Table(size_t R, size_t C) { Init(R, C); }
		~SMMYSQL_Table() { this->Free(); }

		void Free()
		{
			if (this->Cell != nullptr)
			{
				for (size_t i = 0; i < this->Rows; i++)
				{
					DELETE_ARRAY_OBJECT(this->Cell[i]);
				}
				DELETE_ARRAY_OBJECT(this->Cell);
			}

			this->Cols = this->Rows = 0;
		}

		void Init(size_t R, size_t C)
		{
			this->Free();

			this->Cols = C; this->Rows = R;

			this->Cell = new SMAnsiString * [this->Rows];
			for (size_t i = 0; i < this->Rows; i++)
				this->Cell[i] = new SMAnsiString[this->Cols];
		}
};

using MySQLTablePtr = std::unique_ptr<SMMYSQL_Table>;

MYSQL_RES* sm_mysql_query_v2(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query);
SMMYSQL_Table* sm_mysql_query_v3(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query);
std::vector<MySQLTablePtr> sm_mysql_query_list(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const std::vector<SMAnsiString> &queryList);
bool sm_mysql_query_insert(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query);
uint64_t sm_mysql_query_insert_ret_id(const SMAnsiString &Host, const SMAnsiString &DBUser, const SMAnsiString &DBPassword, const SMAnsiString &DBName, const SMAnsiString &Query);

#endif