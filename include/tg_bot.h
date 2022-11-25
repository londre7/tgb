#ifndef TGBOT_MAIN_HEADER
#define TGBOT_MAIN_HEADER

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#else
//#include <vld.h>
#include <winsock2.h>
#endif
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>
#include "smmysql.h"
#include "smdate.h"
#include "smkeylist.h"
#include "json/jansson.h"
#include "socket.h"
#include "http.h"
#include "utils.h"
#include "user.h"
#include "botapi.h"
#include "botutils.h"
#include "configs.h"
#include "processing_commands.h"
#include "threads.h"
#include "callback.h"
#include "db.h"
#include "botmessages.h"
#include "parser.h"
#include "bitmask.h"
#include "chat.h"

#define TGBOT_APPTITLE				"Telegram Bot"
#define TGBOT_VERSION 				"0.1.1"

#define MAKE_FULL_PATH(filename)	GetExeDir() + filename
#define TGBOT_BOTSTATE_FILE			"state"
#define TGBOT_BOTCONFIG_FILE		"bot.conf"
#define TGBOT_MAINLOG_FILE			"main.log"
#define TGBOT_DBLOG_FILE			"db.log"
#define BOT_CONF_FULL_PATH			MAKE_FULL_PATH(TGBOT_BOTCONFIG_FILE) // полный путь к файлу настроек
#define STATE_FILE_FULL_PATH		MAKE_FULL_PATH(TGBOT_BOTSTATE_FILE) // полный путь к файлу с последним обновлением
#define MAIN_LOG_FULL_PATH			MAKE_FULL_PATH(TGBOT_MAINLOG_FILE)
#define DB_LOG_FULL_PATH			MAKE_FULL_PATH(TGBOT_DBLOG_FILE)

#define DELETE_SINGLE_OBJECT(_mptr) if(_mptr != nullptr) { delete _mptr; _mptr = nullptr; }
#define DELETE_ARRAY_OBJECT(_mptr) if(_mptr != nullptr) { delete[] _mptr; _mptr = nullptr; }

extern SMAnsiString GetExeDir();
extern void InitExeDir(const SMAnsiString& path);
extern uint64_t GetCurrentUpdate();
extern void SetCurrentUpdate(uint64_t current_update);

#endif