#include "tg_bot.h"

// глобальные переменные
SMAnsiString					g_ExeDir;				// путь к исполняемомму файлу
uint64_t						g_CurrentUpdate = 0;	// текущее обработанное обновление

SMAnsiString GetExeDir() { return g_ExeDir; }
uint64_t GetCurrentUpdate() { return g_CurrentUpdate; }
void SetCurrentUpdate(uint64_t current_update) { g_CurrentUpdate = current_update; }
inline void InitExeDir(const SMAnsiString& path) { g_ExeDir = ExtractFilePath(path); }

inline void InitOpenSSL()
{
	// инициализация OpenSSL
	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();
}

#ifdef _WIN32
inline bool InitWinSock()
{
	WSADATA ws;
	if (FAILED(WSAStartup(MAKEWORD(2, 2), &ws)))
	{
		WriteMessage(SYSTEMMSG_WSA_ERROR, TGB_TEXTCOLOR_RED);
		return false;
	}
	return true;
}
#endif // _WIN32

int main(int Argc, char *Argv[])
{
	WriteMessage(TGBOT_APPTITLE " " TGBOT_VERSION, TGB_TEXTCOLOR_WHITE);
	
	// инициализация всего и вся
	InitExeDir(Argv[0]);
	InitOpenSSL();
#ifdef _WIN32
	if (!InitWinSock()) return 0;
#endif

	//WriteMessage(SMDateTime(2022, 3, 13, 1, 35, 0).LowLevelTime);

	// загружаем настройки
	BotConfStruct *bc = GetBotConf();
	const SMAnsiString &token = bc->GetParam(BotConfStruct::Token);
	const SMAnsiString &longpoll_timeout = bc->GetParam(BotConfStruct::LongpollTimeout);
	if (!LoadConfFile(GetBotConf(), BOT_CONF_FULL_PATH))
	{
		WriteMessage(SYSTEMMSG_LOADCONF_FAILED, TGB_TEXTCOLOR_YELLOW);
		WriteFormatMessage(SYSTEMMSG_LOADCONF_DEFAULT_PARAMS, TGB_TEXTCOLOR_DEFAULT, C_STR(token), C_STR(longpoll_timeout));
	}
	else WriteFormatMessage(SYSTEMMSG_LOADCONF_LOADED, TGB_TEXTCOLOR_DEFAULT, C_STR(token), C_STR(longpoll_timeout));

	// проверяем, жив ли бот
	std::unique_ptr<TGBOT_User> botinfo(tgbot_getMe());
	if (!botinfo)
	{
		WriteMessage(SYSTEMMSG_CHECKBOT_FAILED, TGB_TEXTCOLOR_RED);
		return 0;
	}
	WriteFormatMessage(SYSTEMMSG_CHECKBOT_OK, TGB_TEXTCOLOR_GREEN, botinfo->Id, C_STR(botinfo->FirstName), C_STR(botinfo->Username));
	
	// загружаем последнее состояние
	if(!LoadLastBotState(STATE_FILE_FULL_PATH))
		WriteMessage(SYSTEMMSG_LOADLASTBOTSTATE_FAILED, TGB_TEXTCOLOR_YELLOW);
	
	// создаём потоки и очередь для каждого потока-обработчика
	const int num_update_threads = GetNumUpdThreads();
	CreateUpdateThreads(num_update_threads);
	WriteFormatMessage(SYSTEMMSG_UPD_THR_STARTED, TGB_TEXTCOLOR_LIGHTBLUE, num_update_threads);
	
	// всё норм, запускаем основной цикл обработки обновлений
	WriteMessage(SYSTEMMSG_PROC_UPD_STARTED);
	TGBOT_Update* update(nullptr);
	std::vector<TGBOT_Update*> updates;
	while(true)
	{
		// получаем обновления V2
		if (!tgbot_GetUpdates(updates, GetCurrentUpdate()+1))
		{
			usleep(5000000);
			continue;
		}
		const size_t numUpdates = updates.size();
		if (!numUpdates)
			continue;

		// парсим обновления
		for(size_t i=0; i< numUpdates; i++)
		{
			update = updates.at(i);
			SetCurrentUpdate(update->UpdateId);

			// сохраняем текущее состояние
			SaveLastBotState(STATE_FILE_FULL_PATH);
			
			// помещаем в нужный поток
			// в случае успешного помещения в поток - память освободится потоком, иначе стираем тут
			if(!PushToUpdQueue(update))
				DELETE_SINGLE_OBJECT(update);
		}
	}
	
	return 0;
}