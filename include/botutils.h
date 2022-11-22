#ifndef TGBOT_BOTUTILS_HEADER
#define TGBOT_BOTUTILS_HEADER

#include "tg_bot.h"

// структура для создания Inline-клавиатуры
struct InlineKeyboardDef
{
	const char* caption; // заголовок (#newrow для создания строки)
	const char* callback;
	StringList* params; // имена параметров
};

#define SEND_MSG_AND_RETURN(id, msgtext) { tgbot_SendMessage(id, msgtext);return; }
#define SEND_MSG_AND_RETURN_WITH_BTN(chatid, msgtext, btncaption) \
{ \
	TGBOT_ReplyKeyboardMarkup kb_cmd(true, false, true); \
	kb_cmd.CreateButton(btncaption); \
	tgbot_SendMessage(chatid, msgtext, &kb_cmd); \
	return; \
}
#define SEND_MSG_AND_RETURN_PTR(id, msgtext) { tgbot_SendMessage(id, msgtext); return nullptr; }
#define EDIT_MSG_AND_RETURN(id, messageid, msgtext) { tgbot_editMessageText(id, messageid, msgtext, GetMainMenuKeyboard());return; }
#define SHOW_MSG_AND_RETURN(src_is_empty, id, messageid, msgtext) { ShowMessage(src_is_empty, id, messageid, msgtext, GetMainMenuKeyboard());return; }
#define CHECK_USRSTATE_FREE(dbuser, chatid) if(dbuser.State != USRSTATE_FREE) { tgbot_SendMessage(chatid, BOTMSG_USR_NO_STATE_FREE); return; }

SMAnsiString MakeFullUserName(const TGBOT_User* user, bool useUsername=false);
SMAnsiString MakeFullUserName(const DB_User* user, bool useUsername=false);
SMAnsiString MakeUsername(const SMAnsiString& username);

SMAnsiString GetMonthStr(int month);
SMAnsiString GetDayWeekStr(int dw);

void ShowMessage(bool SrcIsEmpty, uint64_t ChatID, uint64_t MessageID, const SMAnsiString& Text, TGBOT_InlineKeyboardMarkup* Keyboard=nullptr);

TGBOT_InlineKeyboardMarkup* MakeInlineKeyboardFromDef(const std::vector<InlineKeyboardDef> &keyboardDef, const std::vector<StringList*>& params);

#endif