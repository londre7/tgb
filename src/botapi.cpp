#include "tg_bot.h"

bool tgbot_GetUpdates(std::vector<TGBOT_Update*> &updates, uint64_t offset)
{
	// получаем обновления
	int          err;
	json_t       *root,
	             * upd_array,
	             * upd_array_root,
	             *json_obj;
	json_error_t error;

	updates.clear();

	BotConfStruct* bot_conf = GetBotConf();
	SMAnsiString get_doc = SMAnsiString::smprintf("/bot%s/getUpdates?timeout=%d", C_STR(bot_conf->GetParam(BotConfStruct::Token)), bot_conf->GetIntParam(BotConfStruct::LongpollTimeout));
	if (offset)
		get_doc += "&offset=" + SMAnsiString(offset);
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Get(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, err));
	
	if (http_resp == NULL)
	{
		WriteMessage("Failed connection to Telegram API", TGB_TEXTCOLOR_RED);
		return false;
	}
	if ((http_resp->ErrCode != "200 OK") || (http_resp->ContentType != "application/json"))
	{
		WriteMessage("Proxy error (" + http_resp->ErrCode + "), retry...", TGB_TEXTCOLOR_RED);
		return false;
	}
	if (http_resp->Content.PosSS("{\"ok\":true,\"result\":[]}") != -1)
	{
		return true;
	}
	// смотрим, есть ли обновки
	//WriteMessage(http_resp->Content, 1);
	root = json_loads(C_STR(http_resp->Content), 0, &error);
	if (root == NULL)
	{
		uint64_t current_update = GetCurrentUpdate();
		SetCurrentUpdate(++current_update);
		WriteMessage("Parse json error", TGB_TEXTCOLOR_RED);
		return false;
	}
	json_obj = json_object_get(root, "ok");
	if (json_boolean_value(json_obj) != true)
	{
		WriteMessage("Response error", TGB_TEXTCOLOR_RED);
		json_decref(root);
		return false;
	}
	// парсим обновления
	upd_array = json_object_get(root, "result");
	size_t upd_array_size = json_array_size(upd_array);
	for (size_t i = 0; i < upd_array_size; i++)
	{
		upd_array_root = json_array_get(upd_array, i);
		updates.push_back(new TGBOT_Update(upd_array_root));
	}
	json_decref(root);
	return true;
}

uint64_t GetChatIDFromUpdate(TGBOT_Update* Upd)
{
	if (Upd->CallbackQuery != nullptr) return Upd->CallbackQuery->Message->Chat->Id;
	if (Upd->Message != nullptr) return Upd->Message->Chat->Id;
	if (Upd->EditedMessage != nullptr) return Upd->EditedMessage->Chat->Id;
	if (Upd->ChannelPost != nullptr) return Upd->ChannelPost->Chat->Id;
	if (Upd->EditedChannelPost != nullptr) return Upd->EditedChannelPost->Chat->Id;
	return 0;
}

TGBOT_InlineKeyboardButton::TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton& b)
{
	this->Text = b.Text;
	this->Url = b.Url;
	this->CallbackData = b.CallbackData;
	this->SwitchInlineQuery = b.SwitchInlineQuery;
	this->SwitchInlineQueryCurrentChat = b.SwitchInlineQueryCurrentChat;
	this->Pay = b.Pay;
}

TGBOT_InlineKeyboardMarkup::TGBOT_InlineKeyboardMarkup(const TGBOT_InlineKeyboardMarkup& kb)
{
	this->Init();
	this->CurrentRow = kb.CurrentRow;
	for (int i = 0; i < 16; i++)
	{
		this->NumButtons[i] = kb.NumButtons[i];
		for (int k = 0; k < 8; k++)
		{
			if (kb.Buttons[i][k] != NULL) this->Buttons[i][k] = new TGBOT_InlineKeyboardButton(*kb.Buttons[i][k]);
		}
	}
}

SMAnsiString TGBOT_ReplyKeyboardMarkup::ToJSON()
{
	SMAnsiString	ret;
	SMAnsiString	resize_keyboard,
		one_time_keyboard,
		selective;

	if (this->ResizeKeyboard) resize_keyboard = "true";
	else resize_keyboard = "false";
	if (this->OneTimeKeyboard) one_time_keyboard = "true";
	else one_time_keyboard = "false";
	if (this->Selective) selective = "true";
	else selective = "false";

	ret = "{\"keyboard\":[";
	for (int i = 0; i <= this->CurrentRow; i++)
	{
		if (this->NumButtons[i] == 0) return "";

		ret = ret + SMAnsiString("[");
		for (int j = 0; j < this->NumButtons[i]; j++)
		{
			ret = ret + SMAnsiString("{\"text\":\"") + this->Buttons[i][j]->Text + SMAnsiString("\"}");
			if (j != this->NumButtons[i] - 1) ret = ret + ",";
		}
		ret = ret + SMAnsiString("]");
		if (i != this->CurrentRow) ret = ret + ",";
	}
	ret = ret + SMAnsiString("],\"resize_keyboard\":") + resize_keyboard + SMAnsiString(",\"one_time_keyboard\":") + one_time_keyboard + SMAnsiString(",\"selective\":") + selective + SMAnsiString("}");

	return ret;
}

SMAnsiString TGBOT_InlineKeyboardMarkup::ToJSON()
{
	SMAnsiString	ret;

	ret = "{\"inline_keyboard\":[";
	for (int i = 0; i <= this->CurrentRow; i++)
	{
		if (this->NumButtons[i] == 0) return "";

		ret = ret + SMAnsiString("[");
		for (int j = 0; j < this->NumButtons[i]; j++)
		{
			ret = ret + this->Buttons[i][j]->ToJSON();
			if (j != this->NumButtons[i] - 1) ret = ret + ",";
		}
		ret = ret + SMAnsiString("]");
		if (i != this->CurrentRow) ret = ret + ",";
	}
	ret = ret + SMAnsiString("]}");

	return ret;
}

void tgbot_answerCallbackQuery(SMAnsiString CallbackQueryID)
{
	int 						err;
	SMAnsiString				get_doc,
		content;

	// отправляем сообщение
	BotConfStruct* bot_conf = GetBotConf();
	get_doc.smprintf_s("/bot%s/answerCallbackQuery", C_STR(bot_conf->GetParam(BotConfStruct::Token)));
	content = SMAnsiString("{\"callback_query_id\":\"") + CallbackQueryID + SMAnsiString("\"}");
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, "application/json\0", (void*)C_STR(content), content.length(), err));
}

static HTTP_Response* tgbot_method(const SMAnsiString& method, const SMAnsiString& content)
{
	int err;
	HTTP_Response* http_resp;
	
	// берём всё необходимое из настроек
	BotConfStruct* bot_conf = GetBotConf();
	SMAnsiString tg_token = bot_conf->GetParam(BotConfStruct::Token);
	SMAnsiString tg_host = bot_conf->GetParam(BotConfStruct::TelegramHost);
	int tg_port = bot_conf->GetIntParam(BotConfStruct::TelegramPort);
	int tg_useSSL = bot_conf->GetIntParam(BotConfStruct::UseSSL);

	//делаем запрос
	SMAnsiString get_doc;
	get_doc.smprintf_s("/bot%s/%s", C_STR(tg_token), C_STR(method));
	if(content.IsEmpty())
		http_resp = HTTP_Get(tg_host, tg_port, bool(tg_useSSL), tg_host, get_doc, err);
	else
		http_resp = HTTP_Post(tg_host, tg_port, bool(tg_useSSL), tg_host, get_doc, "application/json\0", (void*)C_STR(content), content.length(), err);

	return http_resp;
}

TGBOT_User* tgbot_getMe()
{
	std::unique_ptr<HTTP_Response> response(tgbot_method("getMe", ""));
	if (!response) return nullptr;

	// парсим json
	json_error_t error;
	json_t *root = json_loads(response->Content, 0, &error);
	if (!root) return nullptr;

	json_t *json_obj = json_object_get(root, "ok");
	if (json_boolean_value(json_obj) != true)
	{
		json_decref(root);
		return nullptr;
	}

	json_obj = json_object_get(root, "result");
	TGBOT_User *usr = new TGBOT_User(json_obj);
	json_decref(root);
	return usr;
}

void tgbot_SendMessage(uint64_t ChatID, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"parse_mode\":\"HTML\", \"text\":\"%s\"%s }\0", 
		ChatID, 
		C_STR(Text), 
		C_STR(((Keyboard)?", \"reply_markup\":"+Keyboard->ToJSON():""))
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendMessage", content));
}

void tgbot_SendMessage(SMKeyList *ChatIDList, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard)
{
	if (!ChatIDList) return;
	const size_t list_count = ChatIDList->size();
	for (size_t i = 0; i < list_count; i++)
		tgbot_SendMessage(ChatIDList->at(i), Text, Keyboard);
}

void tgbot_forwardMessage(uint64_t ChatID, uint64_t FromChatID, uint64_t MessageID)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"from_chat_id\":%llu, \"message_id\":%llu }\0",
		ChatID,
		FromChatID,
		MessageID
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("forwardMessage", content));
}

void tgbot_SendPhotoWithUpload(SMAnsiString Filename, uint64_t ChatID, const SMAnsiString & Caption)
{
	unsigned int	file_size;		// размер файла
	struct stat 	st;
	SMAnsiString	part;
	SMAnsiString	part_delimiter;
	char* filebuf;
	std::ifstream	is;
	int 			err;
	SMAnsiString	get_doc,
		content;

	BotConfStruct* bot_conf = GetBotConf();
	part_delimiter = "6496723c";

	// размер файла
	stat(C_STR(Filename), &st);
	file_size = st.st_size;

	// считываем файл
	filebuf = new char[file_size];
	is.open(C_STR(Filename), std::ios::binary);
	is.read(reinterpret_cast<char*>(filebuf), file_size);
	is.close();

	SMOutBuffer send_content(file_size+1024);

	get_doc = SMAnsiString("/bot") + bot_conf->GetParam(BotConfStruct::Token) + SMAnsiString("/sendPhoto");

	content = SMAnsiString("--") + part_delimiter + SMAnsiString("\r\n");
	content = content + SMAnsiString("Content-Disposition: form-data; name=\"chat_id\"\r\n");
	content = content + SMAnsiString("\r\n");
	content = content + SMAnsiString(ChatID);
	content = content + SMAnsiString("\r\n");

	content = content + SMAnsiString("--") + part_delimiter + SMAnsiString("\r\n");
	content = content + SMAnsiString("Content-Disposition: form-data; name=\"photo\"; filename=\"ph.jpg\"\r\n");
	content = content + SMAnsiString("Content-Type: image/jpeg\r\n");
	content = content + SMAnsiString("Content-Transfer-Encoding: binary\r\n");
	content = content + SMAnsiString("\r\n");

	send_content.PutBlock(C_STR(content), content.length());
	send_content.PutBlock(filebuf, file_size);
	content = SMAnsiString("\r\n");

	content += SMAnsiString("--") + part_delimiter + SMAnsiString("\r\n");
	content = content + SMAnsiString("Content-Disposition: form-data; name=\"caption\"\r\n");
	content = content + SMAnsiString("\r\n");
	content = content + Caption;
	content = content + SMAnsiString("\r\n");

	content = content + SMAnsiString("--") + part_delimiter + SMAnsiString("--\r\n\0");
	send_content.PutBlock(C_STR(content), content.length());

	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, "multipart/form-data; boundary=6496723c\0", (void*)send_content.GetBufferPtr(), send_content.GetWritePos() + 1, err));
	DELETE_ARRAY_OBJECT(filebuf);
}

void tgbot_SendPhotoByFileId(const SMAnsiString & FileID, uint64_t ChatID, const SMAnsiString & Caption)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"photo\":\"%s\", \"caption\":\"%s\" }\0",
		ChatID,
		C_STR(FileID),
		C_STR(Caption)
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendPhoto", content));
}

void tgbot_SendDocument(const char *buf, size_t buflen, const SMAnsiString &outFileName, const SMAnsiString &contentType, uint64_t ChatID, const SMAnsiString& Caption)
{
	int 			err;
	SMAnsiString	get_doc;
	SMAnsiString    content;

	BotConfStruct* bot_conf = GetBotConf();
	const char *part_delimiter = "6496723c";
	SMOutBuffer send_content(buflen + 1024);

	get_doc.smprintf_s("/bot%s/sendDocument", C_STR(bot_conf->GetParam(BotConfStruct::Token)));
	content.smprintf_s
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"chat_id\"\r\n"
		"\r\n"
		"%llu"
		"\r\n",
		part_delimiter, ChatID
	);
	content += SMAnsiString::smprintf
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"document\"; filename=\"%s\"\r\n"
		"Content-Type: %s\r\n"
		"Content-Transfer-Encoding: binary\r\n"
		"\r\n",
		part_delimiter, C_STR(outFileName), C_STR(contentType)
	);
	send_content.PutBlock(C_STR(content), content.length());
	send_content.PutBlock(buf, buflen);
	content = "\r\n";
	content += SMAnsiString::smprintf
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"caption\"\r\n"
		"\r\n"
		"%s"
		"\r\n",
		part_delimiter, C_STR(Caption)
	);
	content += SMAnsiString::smprintf("--%s--\r\n\0", part_delimiter);

	send_content.PutBlock(C_STR(content), content.length());
	
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, SMAnsiString::smprintf("multipart/form-data; boundary=%s", part_delimiter), (void*)send_content.GetBufferPtr(), send_content.GetWritePos() + 1, err));
}

void tgbot_editMessageText(uint64_t chat_id, uint64_t message_id, const SMAnsiString& text, TGBOT_InlineKeyboardMarkup* keyboard)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"message_id\":%llu, \"parse_mode\":\"HTML\", \"text\":\"%s\"%s }\0", 
		chat_id, 
		message_id, 
		C_STR(text), 
		C_STR(((keyboard)?", \"reply_markup\":"+keyboard->ToJSON():""))
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("editMessageText", content));
}

void tgbot_sendVenue(uint64_t chat_id, double latitude, double longitude, const SMAnsiString& title, const SMAnsiString& address, TGBOT_InlineKeyboardMarkup* keyboard)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"latitude\":%s, \"longitude\":%s, \"title\":\"%s\", \"address\":\"%s\"%s }\0", 
		chat_id, 
		C_STR(SMAnsiString(latitude)),
		C_STR(SMAnsiString(longitude)),
		C_STR(title),
		C_STR(address),
		C_STR(((keyboard) ? ", \"reply_markup\":" + keyboard->ToJSON() : ""))
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendVenue", content));
}

void tgbot_deleteMessage(uint64_t chat_id, uint64_t message_id)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"message_id\":%llu }\0",
		chat_id,
		message_id
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("deleteMessage", content));
}

void tgbot_sendSticker(uint64_t chat_id, const SMAnsiString &sticker)
{
	SMAnsiString content;
	content.smprintf_s
	(
		"{ \"chat_id\":%llu, \"sticker\":\"%s\" }\0",
		chat_id,
		C_STR(sticker)
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendSticker", content));
}