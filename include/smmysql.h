#ifndef SM_MYSQL_UTILS
#define SM_MYSQL_UTILS

#include <mysql/mysql.h>
#include <memory>
#include "smstring.h"

#define DELETE_SINGLE_OBJECT(_mptr) if(_mptr != nullptr) { delete _mptr; _mptr = nullptr; }
#define DELETE_ARRAY_OBJECT(_mptr) if(_mptr != nullptr) { delete[] _mptr; _mptr = nullptr; }

class SMMYSQL_Table
{
	public:
		SMAnsiString** Cell;	// €чейки таблицы
		size_t			Cols,
			Rows;

		SMMYSQL_Table() : Cell(NULL), Cols(0), Rows(0) {}
		SMMYSQL_Table(size_t R, size_t C) : Cell(NULL) { Init(R, C); }
		~SMMYSQL_Table() { this->Free(); }

		void Free()
		{
			if (this->Cell != NULL)
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

bool sm_mysql_query(MYSQL* Connection, SMAnsiString Query);
MYSQL_RES* sm_mysql_query_v2(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString& Query);
SMMYSQL_Table* sm_mysql_query_v3(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString& Query);
bool sm_mysql_query_insert(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString& Query);
uint64_t sm_mysql_query_insert_ret_id(SMAnsiString Host, SMAnsiString DBUser, SMAnsiString DBPassword, SMAnsiString DBName, const SMAnsiString& Query);

#endif