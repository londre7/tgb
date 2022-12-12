#ifndef TGBOT_CONFIGS_HEADER
#define TGBOT_CONFIGS_HEADER

#include "tg_bot.h"

enum CONFPARAMTYPE
{
	CPTYPE_INT,
	CPTYPE_STRING
};
struct CONFPARAM { int Index; const char *Name; CONFPARAMTYPE Type; SMAnsiString Value; };

class BaseConf
{
	public:
		virtual void SetParam(const SMAnsiString &Key, const SMAnsiString &Val, CONFPARAM* StructureDef=NULL, size_t StructureDefNum=0)
		{
			if (!StructureDef) return;
			for (int i = 0; i < StructureDefNum; i++)
			{
				if (Key == StructureDef[i].Name)
				{
					StructureDef[i].Value = Val;
					break;
				}
			}
		}
		virtual const SMAnsiString& GetParam(int Index, CONFPARAM* StructureDef, size_t StructureDefNum)
		{
			static SMAnsiString empty("");
			if (!StructureDef) return empty;
			if (Index < StructureDefNum)
				return StructureDef[Index].Value;
			else
				return empty;
		}
		virtual int GetIntParam(int Index, CONFPARAM* StructureDef, size_t StructureDefNum)
		{
			if (!StructureDef) return 0;
			if (Index < StructureDefNum)
				return StructureDef[Index].Value;
			else
				return 0;
		}
};

#define CONF_PARAM_METHODS(def) \
void SetParam(const SMAnsiString &Key, const SMAnsiString &Val, CONFPARAM* StructureDef=NULL, size_t StructureDefNum=0) \
{ BaseConf::SetParam(Key, Val, def, SizeOfArray(def)); } \
const SMAnsiString& GetParam(int Index, CONFPARAM* StructureDef=NULL, size_t StructureDefNum=0) \
{ return BaseConf::GetParam(Index, def, SizeOfArray(def)); } \
int GetIntParam(int Index, CONFPARAM* StructureDef=NULL, size_t StructureDefNum=0) \
{ return BaseConf::GetIntParam(Index, def, SizeOfArray(def)); }

extern bool LoadConfFile(BaseConf* ConfStruct, const SMAnsiString& FileName);

class BotConfStruct : public BaseConf
{
	public:
		enum // список параметров
		{
			Token,            // токен для подключения к боту
			LongpollTimeout,  // время между опросами Telegram API
			TelegramHost,     // хост для подключения к Telegram Bot API
			TelegramPort,     // порт для подключения к Telegram Bot API
			UseSSL,           // использовать SSL при подключениях
			NumUpdateThreads, // количество потоков-обработчиков обновлений
			DBHost,           // хост БД
			DBUser,           // логин для авторизации на сервере БД
			DBPassword,       // пароль для авторизации на сервере БД
			DBDatabase,       // название БД
			UseLog,           // пока не используется
			SuperUser,        // пользоваетель, который имеет доступ ко всем командам
			NUMPARAM
		};
		CONFPARAM ConfStructureDef[NUMPARAM] =
		{
			// параметр         параметр в файле      тип данных     значение по-умолч.
			{ Token,            "token",              CPTYPE_STRING, ""                 },
			{ LongpollTimeout,  "longpoll_timeout",   CPTYPE_INT,    "30"               },
			{ TelegramHost,     "telegram_api_host",  CPTYPE_STRING, "api.telegram.org" },
			{ TelegramPort,     "telegram_api_port",  CPTYPE_INT,    "443"              },
			{ UseSSL,           "use_ssl",            CPTYPE_INT,    "1"                },
			{ NumUpdateThreads, "num_update_threads", CPTYPE_INT,    "0"                },
			{ DBHost,           "db_host",            CPTYPE_STRING, "localhost"        },
			{ DBUser,           "db_user",            CPTYPE_STRING, "root"             },
			{ DBPassword,       "db_password",        CPTYPE_STRING, ""                 },
			{ DBDatabase,       "db_database",        CPTYPE_STRING, "tgbot"            },
			{ UseLog,           "use_log",            CPTYPE_STRING, ""                 },
			{ SuperUser,        "superuser",          CPTYPE_STRING, ""                 },
		};
		CONF_PARAM_METHODS(ConfStructureDef)
};
extern BotConfStruct* GetBotConf();

#endif