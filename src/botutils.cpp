#include "tg_bot.h"

static SMAnsiString MakeFullUserName(const SMAnsiString& lastname, const SMAnsiString& firstname, const SMAnsiString &username)
{
	//return lastname + SMAnsiString((!lastname.IsEmpty()) ? " " : "") + firstname + SMAnsiString((!username.IsEmpty())?SMAnsiString::smprintf(" (%s)", C_STR(MakeUsername(username))):"");
	SMAnsiString fn;
	if (!lastname.IsEmpty()) fn += (lastname + " ");
	fn += firstname;
	if (!username.IsEmpty()) fn += SMAnsiString::smprintf(" (%s)", C_STR(username));
	return fn;
}

SMAnsiString MakeFullUserName(const TGBOT_User* user, bool useUsername)
{
	return MakeFullUserName(user->LastName, user->FirstName, (useUsername)?user->Username:"");
}

SMAnsiString MakeFullUserName(const DB_User* user, bool useUsername)
{
	return MakeFullUserName(user->LastName, user->FirstName, (useUsername)?user->Username:"");
}

SMAnsiString MakeUsername(const SMAnsiString& username)
{
	return (!username.IsEmpty())?SMAnsiString::smprintf("@%s", C_STR(username)):"";
}

SMAnsiString GetMonthStr(int month)
{
	static StringList months =
	{
		"Январь",
		"Февраль",
		"Март",
		"Аперль",
		"Май",
		"Июнь",
		"Июль",
		"Август",
		"Сентябрь",
		"Октябрь",
		"Ноябрь",
		"Декабрь"
	};
	return months.at(month-1);
}
SMAnsiString GetDayWeekStr(int dw)
{
	static StringList dws =
	{
		"Пн",
		"Вт",
		"Ср",
		"Чт",
		"Пт",
		"Сб",
		"Вс"
	};
	return dws.at(dw);
}

void ShowMessage(bool SrcIsEmpty, uint64_t ChatID, uint64_t MessageID, const SMAnsiString& Text, TGBOT_InlineKeyboardMarkup* Keyboard)
{
	if (SrcIsEmpty)
	{
		tgbot_deleteMessage(ChatID, MessageID);
		tgbot_SendMessage(ChatID, Text, Keyboard);
	}
	else
		tgbot_editMessageText(ChatID, MessageID, Text, Keyboard);
}

TGBOT_InlineKeyboardMarkup* MakeInlineKeyboardFromDef(std::vector<InlineKeyboardDef> &keyboardDef, const std::vector<StringList*> &params)
{
	// #newrow
	const size_t numbuttons = keyboardDef.size();
	const size_t numparams = params.size();
	if (!numbuttons || !numparams)
		return nullptr;

	TGBOT_InlineKeyboardMarkup* kb = new TGBOT_InlineKeyboardMarkup;

	// генерим callbackdata
	StringList callbackData;
	std::vector<StringList*> pnames;
	for (size_t i = 0, ri = 0; i < numbuttons; i++)
	{
		if (strcmp(keyboardDef.at(i).caption,"#newrow")==0) { kb->CreateRow(); continue; }
		callbackData.push_back(MakeCallbackData(keyboardDef.at(i).callback));
		pnames.push_back(keyboardDef.at(i).params);
		kb->CreateButton(keyboardDef.at(i).caption, callbackData.at(ri++), false);
	}

	// вбиваем в базу
	RegisterCallbackData(callbackData, pnames, params);
	return kb;
}