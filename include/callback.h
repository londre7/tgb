#ifndef TGBOT_CALLBACK_HEADER
#define TGBOT_CALLBACK_HEADER

#include "tg_bot.h"

// список callback'ов
#define CALLBACK_PRIVATE_POLICY        "private_policy"
#define CALLBACK_CMDLIST               "cmdlist"
#define CALLBACK_FIND                  "find"
#define CALLBACK_ID                    "id"

// параметры для callback'ов
extern StringList CallbackParamsDef_About;

SMAnsiString MakeCallbackData(const SMAnsiString &type, StringList *keys=NULL, StringList *values=NULL);
bool RegisterCallbackData(const StringList& callback, const std::vector<StringList*>& params, const std::vector<StringList*>& values);

// обработчики
void RunCallbackProc(TGBOT_CallbackQuery* RecvCallback, DB_User &dbusrinfo);
#define CQ_PROCESSING_PARAMS DB_User& dbusrinfo, TGBOT_User* From, TGBOT_Message* Message, const SMAnsiString& BtnCaption, const StringList& Params
void cq_processing_private_policy(CQ_PROCESSING_PARAMS);
void cq_processing_cmdlist(CQ_PROCESSING_PARAMS);
void cq_processing_find(CQ_PROCESSING_PARAMS);
void cq_processing_id(CQ_PROCESSING_PARAMS);

#endif