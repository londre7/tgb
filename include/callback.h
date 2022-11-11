#ifndef TGBOT_CALLBACK_HEADER
#define TGBOT_CALLBACK_HEADER

#include "tg_bot.h"

// список callback'ов
#define CALLBACK_PRIVATE_POLICY        "private_policy"
#define CALLBACK_ABOUT                 "about"

struct CallbackParam { SMAnsiString key, value; };

// параметры для callback'ов
extern StringList CallbackParamsDef_About;

SMAnsiString MakeCallbackData(const SMAnsiString &type, StringList *keys=NULL, StringList *values=NULL);
bool RegisterCallbackData(const StringList& callback, const std::vector<StringList*>& params, const std::vector<StringList*>& values);

// обработчики
void RunCallbackProc(TGBOT_CallbackQuery* RecvCallback, const DB_User &dbusrinfo);
#define CQ_PROCESSING_PARAMS const DB_User& dbusrinfo, TGBOT_User* From, TGBOT_Message* Message, const SMAnsiString& BtnCaption, const StringList& Params
void cq_processing_private_policy(CQ_PROCESSING_PARAMS);
void cq_processing_about(CQ_PROCESSING_PARAMS);

#endif