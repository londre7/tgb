#include "tg_bot.h"

#define DECLARE_DEFAULT_CONSTRUCTORS(classname) \
classname::classname() { this->InitAll(); }                                                 \
classname::classname(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); } 
#define DECLARE_DEFAULT_DESTRUCTORS(classname) \
classname::~classname() { this->FreeAll(); }

DECLARE_DEFAULT_CONSTRUCTORS(TGBOT_Chat)
DECLARE_DEFAULT_DESTRUCTORS(TGBOT_Chat)
void TGBOT_Chat::InitAll()
{
	RESET_ULONGLONG(Id);
	CLEAR_STR(Type);
	CLEAR_STR(Title);
	CLEAR_STR(Username);
	CLEAR_STR(FirstName);
	CLEAR_STR(LastName);
	RESET_BOOL(IsForum);
	RESET_PTR(Photo);
	CLEAR_STR(EmojiStatusCustomEmojiId);
	CLEAR_STR(Bio);
	RESET_BOOL(HasPrivateForwards);
	RESET_BOOL(HasRestrictedVoiceAndVideoMessages);
	RESET_BOOL(JoinToSendMessages);
	RESET_BOOL(JoinByRequest);
	CLEAR_STR(Description);
	CLEAR_STR(InviteLink);
	RESET_PTR(PinnedMessage);
	RESET_PTR(Permissions);
	RESET_INT(SlowModeDelay);
	RESET_INT(MessageAutoDeleteTime);
	RESET_BOOL(HasProtectedContent);
	CLEAR_STR(StickerSetName);
	RESET_BOOL(CanSetStickerSet);
	RESET_ULONGLONG(LinkedChatId);
	RESET_PTR(Location);
}
void TGBOT_Chat::FreeAll()
{
	ActiveUsernames.clear();
	DELETE_SINGLE_OBJECT(Photo);
	DELETE_SINGLE_OBJECT(PinnedMessage);
	DELETE_SINGLE_OBJECT(Permissions);
	DELETE_SINGLE_OBJECT(Location);
}
DECLARE_DEFAULT_CONSTRUCTORS(TGBOT_Message)
DECLARE_DEFAULT_DESTRUCTORS(TGBOT_Message)
void TGBOT_Message::InitAll()
{
	RESET_ULONGLONG(MessageId);
	RESET_ULONGLONG(MessageThreadId);
	RESET_PTR(From);
	RESET_PTR(SenderChat);
	RESET_INT(Date);
	RESET_PTR(Chat);
	RESET_PTR(ForwardFrom);
	RESET_PTR(ForwardFromChat);
	RESET_ULONGLONG(ForwardFromMessageId);
	CLEAR_STR(ForwardSignature);
	CLEAR_STR(ForwardSenderName);
	RESET_INT(ForwardDate);
	RESET_BOOL(IsTopicMessage);
	RESET_BOOL(IsAutomaticForward);
	RESET_PTR(ReplyToMessage);
	RESET_PTR(ViaBot);
	RESET_INT(EditDate);
	RESET_BOOL(HasProtectedContent);
	CLEAR_STR(MediaGroupId);
	CLEAR_STR(AuthorSignature);
	CLEAR_STR(Text);
	RESET_PTR(Animation);
	RESET_PTR(Audio);
	RESET_PTR(Document);
	RESET_PTR(Sticker);
	RESET_PTR(Video);
	RESET_PTR(VideoNote);
	RESET_PTR(Voice);
	CLEAR_STR(Caption);
	RESET_PTR(Contact);
	RESET_PTR(Dice);
	RESET_PTR(Game);
	RESET_PTR(Poll);
	RESET_PTR(Venue);
	RESET_PTR(Location);
	CLEAR_STR(NewChatTitle);
	RESET_PTR(LeftChatMember);
	RESET_BOOL(DeleteChatPhoto);
	RESET_BOOL(GroupChatCreated);
	RESET_BOOL(SupergroupChatCreated);
	RESET_BOOL(ChannelChatCreated);
	RESET_PTR(MessageAutoDeleteTimerChanged);
	RESET_ULONGLONG(MigrateToChatId);
	RESET_ULONGLONG(MigrateFromChatId);
	RESET_PTR(PinnedMessage);
	RESET_PTR(Invoice);
	RESET_PTR(SuccessfulPayment);
	RESET_PTR(PassportData);
	RESET_PTR(ProximityAlertTriggered);
	RESET_PTR(ForumTopicCreated);
	RESET_PTR(ReplyMarkup);
	RESET_PTR(ForumTopicClosed);
	RESET_PTR(ForumTopicReopened);
	RESET_PTR(VideoChatScheduled);
	RESET_PTR(VideoChatStarted);
	RESET_PTR(VideoChatEnded);
	RESET_PTR(VideoChatParticipantsInvited);
	RESET_PTR(WebAppData);
}
void TGBOT_Message::FreeAll()
{
	Entities.clear();
	Photo.clear();
	CaptionEntities.clear();
	NewChatMembers.clear();
	NewChatPhoto.clear();
	DELETE_SINGLE_OBJECT(Chat);
	DELETE_SINGLE_OBJECT(SenderChat);
	DELETE_SINGLE_OBJECT(ForwardFrom);
	DELETE_SINGLE_OBJECT(ForwardFromChat);
	DELETE_SINGLE_OBJECT(ReplyToMessage);
	DELETE_SINGLE_OBJECT(ViaBot);
	DELETE_SINGLE_OBJECT(From);
	DELETE_SINGLE_OBJECT(Contact);
	DELETE_SINGLE_OBJECT(Sticker);
	DELETE_SINGLE_OBJECT(ReplyMarkup);
	DELETE_SINGLE_OBJECT(Animation);
	DELETE_SINGLE_OBJECT(Audio);
	DELETE_SINGLE_OBJECT(Document);
	DELETE_SINGLE_OBJECT(Video);
	DELETE_SINGLE_OBJECT(VideoNote);
	DELETE_SINGLE_OBJECT(Voice);
	DELETE_SINGLE_OBJECT(Dice);
	DELETE_SINGLE_OBJECT(Game);
	DELETE_SINGLE_OBJECT(Poll);
	DELETE_SINGLE_OBJECT(Venue);
	DELETE_SINGLE_OBJECT(Location);
	DELETE_SINGLE_OBJECT(LeftChatMember);
	DELETE_SINGLE_OBJECT(MessageAutoDeleteTimerChanged);
	DELETE_SINGLE_OBJECT(PinnedMessage);
	DELETE_SINGLE_OBJECT(Invoice);
	DELETE_SINGLE_OBJECT(SuccessfulPayment);
	DELETE_SINGLE_OBJECT(PassportData);
	DELETE_SINGLE_OBJECT(ProximityAlertTriggered);
	DELETE_SINGLE_OBJECT(ForumTopicCreated);
	DELETE_SINGLE_OBJECT(ForumTopicClosed);
	DELETE_SINGLE_OBJECT(ForumTopicReopened);
	DELETE_SINGLE_OBJECT(VideoChatScheduled);
	DELETE_SINGLE_OBJECT(VideoChatStarted);
	DELETE_SINGLE_OBJECT(VideoChatEnded);
	DELETE_SINGLE_OBJECT(VideoChatParticipantsInvited);
	DELETE_SINGLE_OBJECT(WebAppData);
}

bool tgbot_GetUpdates(std::vector<TGBOT_Update*> &updates, uint64_t offset)
{
	// ???????????????? ????????????????????
	int          err;
	json_t       *root,
	             * upd_array,
	             * upd_array_root,
	             *json_obj;
	json_error_t error;

	updates.clear();

	BotConfStruct* bot_conf = GetBotConf();
	SMAnsiString get_doc = SMAnsiString::smprintf
	(
		"/bot%s/getUpdates?timeout=%d",
		C_STR(bot_conf->GetParam(BotConfStruct::Token)),
		bot_conf->GetIntParam(BotConfStruct::LongpollTimeout)
	);
	if (offset)
		get_doc += SMAnsiString::smprintf("&offset=%llu", offset);
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
	// ??????????????, ???????? ???? ??????????????
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
	// ???????????? ????????????????????
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
	return 0ull;
}

TGBOT_User* GetUserFromUpdate(TGBOT_Update* Upd)
{
	if (Upd->CallbackQuery != nullptr) return Upd->CallbackQuery->From;
	if (Upd->Message != nullptr) return Upd->Message->From;
	if (Upd->EditedMessage != nullptr) return Upd->EditedMessage->From;
	if (Upd->ChannelPost != nullptr) return Upd->ChannelPost->From;
	if (Upd->EditedChannelPost != nullptr) return Upd->EditedChannelPost->From;
	return nullptr;
}

TGBOT_KeyboardButton::TGBOT_KeyboardButton(const TGBOT_KeyboardButton* val)
{
	Text = val->Text;
	RequestContact = val->Text;
	RequestLocation = val->RequestLocation;
}

TGBOT_KeyboardButton::TGBOT_KeyboardButton(const TGBOT_KeyboardButton &val)
{
	Text = val.Text;
	RequestContact = val.Text;
	RequestLocation = val.RequestLocation;
}

TGBOT_KeyboardButton::TGBOT_KeyboardButton(SMAnsiString Text, bool RequestContact, bool RequestLocation)
{
	this->Text = Text;
	this->RequestContact = RequestContact;
	this->RequestLocation = RequestLocation;
}

TGBOT_InlineKeyboardButton::TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton *b)
{
	Text = b->Text;
	Url = b->Url;
	CallbackData = b->CallbackData;
	SwitchInlineQuery = b->SwitchInlineQuery;
	SwitchInlineQueryCurrentChat = b->SwitchInlineQueryCurrentChat;
	Pay = b->Pay;
}

TGBOT_InlineKeyboardButton::TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton &b)
{
	Text = b.Text;
	Url = b.Url;
	CallbackData = b.CallbackData;
	SwitchInlineQuery = b.SwitchInlineQuery;
	SwitchInlineQueryCurrentChat = b.SwitchInlineQueryCurrentChat;
	Pay = b.Pay;
}

void TGBOT_InlineKeyboardButton::CustomInit(SMAnsiString Text, SMAnsiString Url, SMAnsiString CallbackData, bool Pay)
{
	this->Text = Text;
	this->CallbackData = CallbackData;
	this->Url = Url;
	this->Pay = Pay;
}

void TGBOT_ReplyKeyboardMarkup::CustomInit(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective, bool freemem)
{
	Keyboard.clear();
	this->ResizeKeyboard = ResizeKeyboard;
	this->OneTimeKeyboard = OneTimeKeyboard;
	this->Selective = Selective;
}

void TGBOT_ReplyKeyboardMarkup::CreateButton(SMAnsiString Text)
{
	Keyboard.push_back(new TGBOT_KeyboardButton(Text, false, false));
}

void tgbot_answerCallbackQuery(const SMAnsiString &CallbackQueryID)
{
	int          err;
	SMAnsiString get_doc,
	             content;

	// ???????????????????? ??????????????????
	BotConfStruct* bot_conf = GetBotConf();
	get_doc.smprintf_s("/bot%s/answerCallbackQuery", C_STR(bot_conf->GetParam(BotConfStruct::Token)));
	content.smprintf_s("{\"callback_query_id\":\"%s\"}", C_STR(CallbackQueryID));
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, "application/json\0", (void*)C_STR(content), content.length(), err));
}

static HTTP_Response* tgbot_method(const SMAnsiString& method, const SMAnsiString& content)
{
	int err;
	HTTP_Response* http_resp;
	
	// ?????????? ?????? ?????????????????????? ???? ????????????????
	BotConfStruct* bot_conf = GetBotConf();
	const SMAnsiString &tg_token = bot_conf->GetParam(BotConfStruct::Token);
	const SMAnsiString &tg_host = bot_conf->GetParam(BotConfStruct::TelegramHost);
	int tg_port = bot_conf->GetIntParam(BotConfStruct::TelegramPort);
	int tg_useSSL = bot_conf->GetIntParam(BotConfStruct::UseSSL);

	//???????????? ????????????
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

	// ???????????? json
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
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"parse_mode\":\"HTML\", \"text\":\"%s\"%s }\0",
		ChatID,
		C_STR(Text),
		C_STR(((Keyboard) ? ", \"reply_markup\":" + Keyboard->ToJSON() : SMAnsiString("")))
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
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"from_chat_id\":%llu, \"message_id\":%llu }\0",
		ChatID,
		FromChatID,
		MessageID
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("forwardMessage", content));
}

void tgbot_SendPhotoWithUpload(const SMAnsiString &Filename, uint64_t ChatID, const SMAnsiString & Caption)
{
	BotConfStruct *bot_conf = GetBotConf();

	// ???????????? ??????????
	struct stat st;
	stat(C_STR(Filename), &st);
	off_t file_size = st.st_size;

	// ?????????????????? ????????
	char *filebuf = new char[file_size];
	std::ifstream is;
	is.open(C_STR(Filename), std::ios::binary);
	is.read(reinterpret_cast<char*>(filebuf), file_size);
	is.close();

	SMOutBuffer send_content(file_size+1024);

	const char *part_delimiter = "6496723c";
	SMAnsiString get_doc = SMAnsiString::smprintf("/bot%s/sendPhoto", C_STR(bot_conf->GetParam(BotConfStruct::Token)));
	SMAnsiString content = SMAnsiString::smprintf
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"chat_id\"\r\n"
		"\r\n%llu\r\n"
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"photo\"; filename=\"ph.jpg\"\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Transfer-Encoding: binary\r\n"
		"\r\n",
		part_delimiter,
		ChatID,
		part_delimiter
	);
	send_content.PutBlock(C_STR(content), content.length());
	send_content.PutBlock(filebuf, file_size);
	content.smprintf_s
	(
		"\r\n"
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"caption\"\r\n"
		"\r\n%s\r\n"
		"--%s--\r\n\0",
		part_delimiter,
		C_STR(Caption),
		part_delimiter
	);
	send_content.PutBlock(C_STR(content), content.length());

	int err;
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, "multipart/form-data; boundary=6496723c\0", (void*)send_content.GetBufferPtr(), send_content.GetWritePos() + 1, err));
	DELETE_ARRAY_OBJECT(filebuf);
}

void tgbot_SendPhotoByFileId(const SMAnsiString & FileID, uint64_t ChatID, const SMAnsiString & Caption)
{
	SMAnsiString content = SMAnsiString::smprintf
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

	BotConfStruct* bot_conf = GetBotConf();
	const char *part_delimiter = "6496723c";
	SMOutBuffer send_content(buflen + 1024);

	SMAnsiString get_doc = SMAnsiString::smprintf
	(
		"/bot%s/sendDocument",
		C_STR(bot_conf->GetParam(BotConfStruct::Token))
	);
	SMAnsiString content = SMAnsiString::smprintf
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"chat_id\"\r\n"
		"\r\n"
		"%llu"
		"\r\n",
		part_delimiter,
		ChatID
	);
	content += SMAnsiString::smprintf
	(
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"document\"; filename=\"%s\"\r\n"
		"Content-Type: %s\r\n"
		"Content-Transfer-Encoding: binary\r\n"
		"\r\n",
		part_delimiter,
		C_STR(outFileName),
		C_STR(contentType)
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
		part_delimiter,
		C_STR(Caption)
	);
	content += SMAnsiString::smprintf("--%s--\r\n\0", part_delimiter);

	send_content.PutBlock(C_STR(content), content.length());
	
	std::unique_ptr<HTTP_Response> http_resp(HTTP_Post(bot_conf->GetParam(BotConfStruct::TelegramHost), bot_conf->GetIntParam(BotConfStruct::TelegramPort), bool(bot_conf->GetIntParam(BotConfStruct::UseSSL)), bot_conf->GetParam(BotConfStruct::TelegramHost), get_doc, SMAnsiString::smprintf("multipart/form-data; boundary=%s", part_delimiter), (void*)send_content.GetBufferPtr(), send_content.GetWritePos() + 1, err));
}

void tgbot_editMessageText(uint64_t chat_id, uint64_t message_id, const SMAnsiString& text, TGBOT_InlineKeyboardMarkup* keyboard)
{
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"message_id\":%llu, \"parse_mode\":\"HTML\", \"text\":\"%s\"%s }\0", 
		chat_id, 
		message_id, 
		C_STR(text), 
		C_STR(((keyboard)?", \"reply_markup\":"+keyboard->ToJSON():SMAnsiString("")))
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("editMessageText", content));
}

void tgbot_sendVenue(uint64_t chat_id, double latitude, double longitude, const SMAnsiString& title, const SMAnsiString& address, TGBOT_InlineKeyboardMarkup* keyboard)
{
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"latitude\":%s, \"longitude\":%s, \"title\":\"%s\", \"address\":\"%s\"%s }\0", 
		chat_id, 
		C_STR(SMAnsiString(latitude)),
		C_STR(SMAnsiString(longitude)),
		C_STR(title),
		C_STR(address),
		C_STR(((keyboard) ? ", \"reply_markup\":" + keyboard->ToJSON() : SMAnsiString("")))
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendVenue", content));
}

void tgbot_deleteMessage(uint64_t chat_id, uint64_t message_id)
{
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"message_id\":%llu }\0",
		chat_id,
		message_id
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("deleteMessage", content));
}

void tgbot_sendSticker(uint64_t chat_id, const SMAnsiString &sticker)
{
	SMAnsiString content = SMAnsiString::smprintf
	(
		"{ \"chat_id\":%llu, \"sticker\":\"%s\" }\0",
		chat_id,
		C_STR(sticker)
	);
	std::unique_ptr<HTTP_Response> response(tgbot_method("sendSticker", content));
}