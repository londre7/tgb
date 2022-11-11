#ifndef TGBOT_BOTAPI_HEADER
#define TGBOT_BOTAPI_HEADER

#include "tg_bot.h"

#define FROM_JSON_GET_VALUE(json_fld, struct_fld, json_type_func) JObj = json_object_get(ObjectEntry, json_fld); if (JObj != NULL) { struct_fld = json_type_func(JObj); }
#define FROM_JSON_GET_OBJECT(json_fld, struct_fld, struct_type) JObj = json_object_get(ObjectEntry, json_fld); if (JObj != NULL) { struct_fld = new struct_type(JObj); }
#define FROM_JSON_GET_OBJECT_ARRAY(json_fld, struct_arr, struct_arr_size, struct_arr_type) \
json_t *JArr = json_object_get(ObjectEntry, json_fld); \
if (JArr != NULL) \
{ \
	struct_arr_size = json_array_size(JArr); \
	for (int i = 0; i < struct_arr_size; i++) \
	{ \
		JObj = json_array_get(JArr, i); \
		struct_arr[i] = new struct_arr_type(JObj); \
	} \
}
#define FROM_JSON_GET_ARRAYOFARRAY(json_fld, struct_fld, struct_rows, struct_cols, struct_type) \
json_t* JArr2; \
json_t* JArr1 = json_object_get(ObjectEntry, json_fld); \
if (JArr1 != NULL) \
{ \
	struct_rows = json_array_size(JArr1); \
	for (int i = 0; i < struct_rows; i++) \
	{ \
		JArr2 = json_array_get(JArr1, i); \
		struct_cols[i] = json_array_size(JArr2); \
		for (int j = 0; j < struct_cols[i]; j++) \
		{ \
			JObj = json_array_get(JArr2, j); \
			struct_fld[i][j] = new struct_type(JObj); \
		} \
	} \
}
#define TO_JSON_SET_START int c=0; SMAnsiString ret = "{"
#define TO_JSON_SET_VALUE(json_fld, struct_fld, is_integer) ret = ret + SMAnsiString((c++)?",":"") + SMAnsiString("\"") + SMAnsiString(json_fld) + SMAnsiString((is_integer)?"\":":"\":\"") + SMAnsiString(struct_fld) + SMAnsiString((is_integer)?"":"\"")
#define TO_JSON_SET_OBJECT(json_fld, struct_fld) if(struct_fld != NULL) { ret = ret + SMAnsiString((c++)?",":"") + SMAnsiString("\"") + SMAnsiString(json_fld) + SMAnsiString("\":") + struct_fld->ToJSON(); }
//#define TO_JSON_SET_OBJECT_IF_EXIST(json_fld, struct_fld) if(struct_fld != NULL) { TO_JSON_SET_OBJECT(json_fld, struct_fld); }
#define TO_JSON_SET_OBJECT_ARRAY(json_fld, struct_arr, struct_arr_size) \
if(struct_arr_size) \
{ \
	ret = ret + SMAnsiString((c++) ? "," : "") + SMAnsiString("\"") + SMAnsiString(json_fld) + "\":["; \
	for (int i = 0; i < struct_arr_size; i++) \
	{ \
		ret = ret + struct_arr[i]->ToJSON() + SMAnsiString((i < (struct_arr_size - 1)) ? "," : ""); \
	} \
	ret = ret + SMAnsiString("]"); \
}
//#define TO_JSON_SET_OBJECT_ARRAY_IF_EXIST(json_fld, struct_arr, struct_arr_size, is_last) if(struct_arr_size) { TO_JSON_SET_OBJECT_ARRAY(json_fld, struct_arr, struct_arr_size, is_last); }
#define TO_JSON_SET_BOOL(json_fld, struct_fld) if(struct_fld) { TO_JSON_SET_VALUE(json_fld, struct_fld, true); }
#define TO_JSON_SET_END ret = ret + "}"; return ret

class TGBOT_PhotoSize
{
	public:
		SMAnsiString FileId,
					 FileUniqueId;
		int          Width,
					 Height;
		long long    FileSize;

		TGBOT_PhotoSize() {}
		TGBOT_PhotoSize(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("file_id", this->FileId, json_string_value);
			FROM_JSON_GET_VALUE("file_unique_id", this->FileUniqueId, json_string_value);
			FROM_JSON_GET_VALUE("width", this->Width, json_integer_value);
			FROM_JSON_GET_VALUE("height", this->Height, json_integer_value);
			FROM_JSON_GET_VALUE("file_size", this->FileSize, json_integer_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("file_id", this->FileId, false);
			TO_JSON_SET_VALUE("file_unique_id", this->FileUniqueId, false);
			TO_JSON_SET_VALUE("width", this->Width, true);
			TO_JSON_SET_VALUE("height", this->Height, true);
			TO_JSON_SET_VALUE("file_size", this->FileSize, true);
			TO_JSON_SET_END;
		}
};

class TGBOT_MaskPosition
{
	public:
		SMAnsiString Point;
		double       XShift,
					 YShift,
					 Scale;

		TGBOT_MaskPosition() {}
		TGBOT_MaskPosition(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("point", this->Point, json_string_value);
			FROM_JSON_GET_VALUE("x_shift", this->XShift, json_real_value);
			FROM_JSON_GET_VALUE("y_shift", this->YShift, json_real_value);
			FROM_JSON_GET_VALUE("scale", this->Scale, json_real_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("point", this->Point, false);
			TO_JSON_SET_VALUE("x_shift", this->XShift, true);
			TO_JSON_SET_VALUE("y_shift", this->YShift, true);
			TO_JSON_SET_VALUE("scale", this->Scale, true);
			TO_JSON_SET_END;
		}
};

class TGBOT_User
{
	public:
		uint64_t     Id;
		bool         Is_Bot;
		bool         IsPremium;
		SMAnsiString FirstName,
					 LastName,
					 Username,
					 LanguageCode;

		TGBOT_User() {}
		TGBOT_User(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("id", this->Id, json_integer_value);
			FROM_JSON_GET_VALUE("is_bot", this->Is_Bot, json_boolean_value);
			FROM_JSON_GET_VALUE("first_name", this->FirstName, json_string_value);
			FROM_JSON_GET_VALUE("last_name", this->LastName, json_string_value);
			FROM_JSON_GET_VALUE("username", this->Username, json_string_value);
			FROM_JSON_GET_VALUE("language_code", this->LanguageCode, json_string_value);
			FROM_JSON_GET_VALUE("is_premium", this->IsPremium, json_boolean_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("id", this->Id, true);
			TO_JSON_SET_VALUE("is_bot", this->Is_Bot, true);
			TO_JSON_SET_VALUE("first_name", this->FirstName, false);
			TO_JSON_SET_VALUE("last_name", this->LastName, false);
			TO_JSON_SET_VALUE("username", this->Username, false);
			TO_JSON_SET_VALUE("language_code", this->LanguageCode, false);
			TO_JSON_SET_VALUE("is_premium", this->IsPremium, true);
			TO_JSON_SET_END;
		}
};

class TGBOT_Sticker
{
	public:
		SMAnsiString        FileId,
		                    FileUniqueId;
		int                 Width,
		                    Height;
		bool                IsAnimated;
		TGBOT_PhotoSize*    Thumb;
		SMAnsiString        Emoji,
		                    SetName;
		TGBOT_MaskPosition* MaskPosition;
		long long           FileSize;


		TGBOT_Sticker() { this->InitAll(); }
		TGBOT_Sticker(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
		~TGBOT_Sticker() { this->FreeAll(); }

		void FreeAll()
		{
			DELETE_SINGLE_OBJECT(this->Thumb);
			DELETE_SINGLE_OBJECT(this->MaskPosition);
		}
		void InitAll()
		{
			this->Thumb = NULL;
			this->MaskPosition = NULL;
		}
		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			this->FreeAll();
			FROM_JSON_GET_VALUE("file_id", this->FileId, json_string_value);
			FROM_JSON_GET_VALUE("file_unique_id", this->FileUniqueId, json_string_value);
			FROM_JSON_GET_VALUE("width", this->Width, json_integer_value);
			FROM_JSON_GET_VALUE("height", this->Height, json_integer_value);
			FROM_JSON_GET_VALUE("is_animated", this->IsAnimated, json_boolean_value);
			FROM_JSON_GET_OBJECT("thumb", this->Thumb, TGBOT_PhotoSize);
			FROM_JSON_GET_VALUE("emoji", this->Emoji, json_string_value);
			FROM_JSON_GET_VALUE("set_name", this->SetName, json_string_value);
			FROM_JSON_GET_OBJECT("mask_position", this->MaskPosition, TGBOT_MaskPosition);
			FROM_JSON_GET_VALUE("file_size", this->FileSize, json_integer_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("file_id", this->FileId, false);
			TO_JSON_SET_VALUE("file_unique_id", this->FileUniqueId, false);
			TO_JSON_SET_VALUE("width", this->Width, true);
			TO_JSON_SET_VALUE("height", this->Height, true);
			TO_JSON_SET_VALUE("is_animated", this->IsAnimated, true);
			TO_JSON_SET_OBJECT("thumb", this->Thumb);
			TO_JSON_SET_VALUE("emoji", this->Emoji, false);
			TO_JSON_SET_VALUE("set_name", this->SetName, false);
			TO_JSON_SET_OBJECT("mask_position", this->MaskPosition);
			TO_JSON_SET_VALUE("file_size", this->FileSize, true);
			TO_JSON_SET_END;
		}
};

class TGBOT_StickerSet
{
	public:
		SMAnsiString   Name,
					   Title;
		bool           IsAnimated,
					   ContainsMasks;
		TGBOT_Sticker* Stickers[128];
		int            NumStickers;


		TGBOT_StickerSet() { this->InitAll(); }
		TGBOT_StickerSet(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
		~TGBOT_StickerSet() { this->FreeAll(); }

		void FreeAll()
		{
			for (int i = 0; i < 128; i++)
				DELETE_SINGLE_OBJECT(this->Stickers[i]);
			this->NumStickers = 0;

		}
		void InitAll()
		{
			for (int i = 0; i < 128; i++)
				this->Stickers[i] = NULL;
			this->NumStickers = 0;
		}
		void FromJSON(json_t * ObjectEntry)
		{
			json_t* JObj;
			this->FreeAll();
			FROM_JSON_GET_VALUE("name", this->Name, json_string_value);
			FROM_JSON_GET_VALUE("title", this->Title, json_string_value);
			FROM_JSON_GET_VALUE("is_animated", this->IsAnimated, json_boolean_value);
			FROM_JSON_GET_VALUE("contains_masks", this->ContainsMasks, json_boolean_value);
			FROM_JSON_GET_OBJECT_ARRAY("stickers", this->Stickers, this->NumStickers, TGBOT_Sticker);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("name", this->Name, false);
			TO_JSON_SET_VALUE("title", this->Title, false);
			TO_JSON_SET_VALUE("is_animated", this->IsAnimated, true);
			TO_JSON_SET_VALUE("contains_masks", this->ContainsMasks, true);
			TO_JSON_SET_OBJECT_ARRAY("stickers", this->Stickers, this->NumStickers);
			TO_JSON_SET_END;
		}
};

class TGBOT_Chat
{
	public:
		uint64_t     Id;
		SMAnsiString Type;
		SMAnsiString Title;
		SMAnsiString Username;
		SMAnsiString FirstName;
		SMAnsiString LastName;

		TGBOT_Chat() {}
		TGBOT_Chat(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("id", this->Id, json_integer_value);
			FROM_JSON_GET_VALUE("type", this->Type, json_string_value);
			FROM_JSON_GET_VALUE("title", this->Title, json_string_value);
			FROM_JSON_GET_VALUE("first_name", this->FirstName, json_string_value);
			FROM_JSON_GET_VALUE("last_name", this->LastName, json_string_value);
			FROM_JSON_GET_VALUE("username", this->Username, json_string_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("id", this->Id, true);
			TO_JSON_SET_VALUE("type", this->Type, false);
			TO_JSON_SET_VALUE("title", this->Title, false);
			TO_JSON_SET_VALUE("first_name", this->FirstName, false);
			TO_JSON_SET_VALUE("last_name", this->LastName, false);
			TO_JSON_SET_VALUE("username", this->Username, false);
			TO_JSON_SET_END;
		}
};

class TGBOT_Contact
{
	public:
		SMAnsiString PhoneNumber,
		             FirstName,
		             LastName;
		uint64_t     UserId;
		SMAnsiString Vcard;

		TGBOT_Contact() {}
		TGBOT_Contact(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("phone_number", this->PhoneNumber, json_string_value);
			FROM_JSON_GET_VALUE("first_name", this->FirstName, json_string_value);
			FROM_JSON_GET_VALUE("last_name", this->LastName, json_string_value);
			FROM_JSON_GET_VALUE("user_id", this->UserId, json_integer_value);
			FROM_JSON_GET_VALUE("vcard", this->Vcard, json_string_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("phone_number", this->PhoneNumber, false);
			TO_JSON_SET_VALUE("first_name", this->FirstName, false);
			TO_JSON_SET_VALUE("last_name", this->LastName, false);
			TO_JSON_SET_VALUE("user_id", this->UserId, true);
			TO_JSON_SET_VALUE("vcard", this->Vcard, false);
			TO_JSON_SET_END;
		}
};

class TGBOT_KeyboardButton
{
	public:
		SMAnsiString Text;
		bool         RequestContact,
		             RequestLocation;

		TGBOT_KeyboardButton() {}
		TGBOT_KeyboardButton(SMAnsiString Text, bool RequestContact, bool RequestLocation)
		{
			Init(Text, RequestContact, RequestLocation);
		}

		void Init(SMAnsiString Text, bool RequestContact, bool RequestLocation)
		{
			this->Text = Text;
			this->RequestContact = RequestContact;
			this->RequestLocation = RequestLocation;
		}

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("text", this->Text, json_string_value);
			FROM_JSON_GET_VALUE("request_contact", this->RequestContact, json_boolean_value);
			FROM_JSON_GET_VALUE("request_location", this->RequestLocation, json_boolean_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("text", this->Text, false);
			TO_JSON_SET_BOOL("request_contact", this->RequestContact);
			TO_JSON_SET_BOOL("request_location", this->RequestLocation);
			TO_JSON_SET_END;
		}
};

class TGBOT_InlineKeyboardButton
{
	public:
		SMAnsiString Text;
		SMAnsiString Url;
		SMAnsiString CallbackData;
		SMAnsiString SwitchInlineQuery;
		SMAnsiString SwitchInlineQueryCurrentChat;
		bool         Pay;

		TGBOT_InlineKeyboardButton() { Init("inline_button", "", "", false); }
		TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton& b);
		TGBOT_InlineKeyboardButton(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }
		TGBOT_InlineKeyboardButton(SMAnsiString Text) { Init(Text, "", "", false); }
		TGBOT_InlineKeyboardButton(SMAnsiString Text, SMAnsiString Url) { Init(Text, Url, "", false); }
		TGBOT_InlineKeyboardButton(SMAnsiString Text, SMAnsiString CallbackData, bool Pay) { Init(Text, "", CallbackData, Pay); }

		void Init(SMAnsiString Text, SMAnsiString Url, SMAnsiString CallbackData, bool Pay)
		{
			this->Text = Text;
			this->CallbackData = CallbackData;
			this->Url = Url;
			this->Pay = Pay;
		}

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("text", this->Text, json_string_value);
			FROM_JSON_GET_VALUE("url", this->Url, json_string_value);
			FROM_JSON_GET_VALUE("callback_data", this->CallbackData, json_string_value);
			FROM_JSON_GET_VALUE("switch_inline_query", this->SwitchInlineQuery, json_string_value);
			FROM_JSON_GET_VALUE("switch_inline_query_current_chat", this->SwitchInlineQueryCurrentChat, json_string_value);
			FROM_JSON_GET_VALUE("pay", this->Pay, json_boolean_value);
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString	ret;
			ret = SMAnsiString("{\"text\":\"") + this->Text + SMAnsiString("\"");
			if (this->Url != "") { ret = ret + SMAnsiString(",\"url\":\"") + this->Url + SMAnsiString("\""); }
			if (this->CallbackData != "") { ret = ret + SMAnsiString(",\"callback_data\":\"") + this->CallbackData + SMAnsiString("\""); }
			if (this->Pay) { ret = ret + SMAnsiString(",\"pay\":true"); }
			ret = ret + "}";
			return ret;
		}
};

class TGBOT_Keyboard
{
	public:
		virtual ~TGBOT_Keyboard() {}
		virtual SMAnsiString ToJSON() = 0;
		virtual void FromJSON(json_t* ObjectEntry) = 0;
};

class TGBOT_ReplyKeyboardMarkup: public TGBOT_Keyboard
{
	private:
		TGBOT_KeyboardButton* Buttons[8][8];
		int                   NumButtons[8];
		int                   CurrentRow;
		bool                  ResizeKeyboard,
		                      OneTimeKeyboard,
		                      Selective;

	public:
		TGBOT_ReplyKeyboardMarkup() { Init(true, false, false, false); }
		TGBOT_ReplyKeyboardMarkup(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective) { Init(ResizeKeyboard, OneTimeKeyboard, Selective, false); }
		virtual ~TGBOT_ReplyKeyboardMarkup() 
		{
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					DELETE_SINGLE_OBJECT(Buttons[i][j]);
				}
			}
		}

		void Init(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective, bool freemem=true)
		{
			for (int i = 0; i < 8; i++)
			{
				NumButtons[i] = 0;
				for (int j = 0; j < 8; j++)
				{
					if (freemem)
					{
						DELETE_SINGLE_OBJECT(Buttons[i][j])
					}
					else
						Buttons[i][j] = nullptr;
				}
			}
			CurrentRow = 0;

			this->ResizeKeyboard = ResizeKeyboard;
			this->OneTimeKeyboard = OneTimeKeyboard;
			this->Selective = Selective;
		}

		void CreateButton(SMAnsiString Text)
		{
			if (NumButtons[CurrentRow] < 8)
				Buttons[CurrentRow][NumButtons[CurrentRow]++] = new TGBOT_KeyboardButton(Text, false, false);
		}
		void CreateRow() { if (CurrentRow < 7) CurrentRow++; }
	
		virtual SMAnsiString ToJSON();
		virtual void FromJSON(json_t* ObjectEntry) {}
};

class TGBOT_InlineKeyboardMarkup: public TGBOT_Keyboard
{
	public:
		TGBOT_InlineKeyboardButton* Buttons[16][8];
		int                         NumButtons[16];
		int                         CurrentRow;

		TGBOT_InlineKeyboardMarkup() { this->Init(); }
		TGBOT_InlineKeyboardMarkup(const TGBOT_InlineKeyboardMarkup& kb);
		TGBOT_InlineKeyboardMarkup(json_t* ObjectEntry) { this->Init(); this->FromJSON(ObjectEntry); }
		virtual ~TGBOT_InlineKeyboardMarkup() { this->FreeAll(); }

		void Init()
		{
			for (int i = 0; i < 16; i++)
			{
				this->NumButtons[i] = 0;
				for (int j = 0; j < 8; j++)
					this->Buttons[i][j] = NULL;
			}
			this->CurrentRow = 0;
		}
		void FreeAll()
		{
			for (int i = 0; i < 16; i++)
			{
				this->NumButtons[i] = 0;
				for (int j = 0; j < 8; j++)
					DELETE_SINGLE_OBJECT(this->Buttons[i][j]);
			}
			this->CurrentRow = 0;
		}

		void CreateButton(SMAnsiString Text)
		{
			if (NumButtons[CurrentRow] < 16)
				Buttons[CurrentRow][NumButtons[CurrentRow]++] = new TGBOT_InlineKeyboardButton(Text);
		}
		void CreateButton(SMAnsiString Text, SMAnsiString Url)
		{
			if (NumButtons[CurrentRow] < 16)
				Buttons[CurrentRow][NumButtons[CurrentRow]++] = new TGBOT_InlineKeyboardButton(Text, Url);
		}
		void CreateButton(SMAnsiString Text, SMAnsiString CallbackData, bool Pay)
		{
			if (NumButtons[CurrentRow] < 16)
				Buttons[CurrentRow][NumButtons[CurrentRow]++] = new TGBOT_InlineKeyboardButton(Text, CallbackData, Pay);
		}
		void CreateRow() { if (CurrentRow < 15) CurrentRow++; }
		virtual void FromJSON(json_t * ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_ARRAYOFARRAY("inline_keyboard", Buttons, CurrentRow, NumButtons, TGBOT_InlineKeyboardButton);
		}
		virtual SMAnsiString ToJSON();
};

class TGBOT_ForceReply
{
	public:
		bool ForceReply,
		     Selective;

		TGBOT_ForceReply() {}
		TGBOT_ForceReply(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FromJSON(json_t* ObjectEntry)
		{
			json_t* JObj;
			FROM_JSON_GET_VALUE("force_reply", this->ForceReply, json_boolean_value);
			FROM_JSON_GET_VALUE("selective", this->Selective, json_boolean_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("force_reply", this->ForceReply, true);
			TO_JSON_SET_BOOL("selective", this->Selective);
			TO_JSON_SET_END;
		}
};

class TGBOT_Message
{
	public:
		uint64_t                    Message_Id;
		TGBOT_User*                 From;
		time_t                      Date;
		TGBOT_Chat*                 Chat;
		TGBOT_User*                 ForwardFrom;
		TGBOT_Chat*                 ForwardFromChat;
		uint64_t                    ForwardFromMessageId;
		SMAnsiString                ForwardSignature;
		SMAnsiString                ForwardSenderName;
		time_t                      ForwardDate;
		TGBOT_Message*              ReplyToMessage;
		time_t                      EditDate;
		SMAnsiString                MediaGroupId;
		SMAnsiString                AuthorSignature;
		SMAnsiString                Text;
		TGBOT_Contact*              Contact;
		TGBOT_Sticker*              Sticker;
		TGBOT_PhotoSize*            Photo[64];
		int                         NumPhoto;
		TGBOT_InlineKeyboardMarkup* ReplyMarkup;

		TGBOT_Message() { this->InitAll(); }
		TGBOT_Message(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
		~TGBOT_Message() { this->FreeAll(); }

		void FreeAll()
		{
			DELETE_SINGLE_OBJECT(this->Chat);
			DELETE_SINGLE_OBJECT(this->ForwardFrom);
			DELETE_SINGLE_OBJECT(this->ForwardFromChat);
			DELETE_SINGLE_OBJECT(this->ReplyToMessage);
			DELETE_SINGLE_OBJECT(this->From);
			DELETE_SINGLE_OBJECT(this->Contact);
			DELETE_SINGLE_OBJECT(this->Sticker);
			for (int i = 0; i < 64; i++)
				DELETE_SINGLE_OBJECT(this->Photo[i]);
			DELETE_SINGLE_OBJECT(this->ReplyMarkup);
			NumPhoto = 0;
		}
		void InitAll()
		{
			this->From = NULL;
			this->Chat = NULL;
			this->ForwardFrom = NULL;
			this->ForwardFromChat = NULL;
			this->ReplyToMessage = NULL;
			this->Contact = NULL;
			this->Sticker = NULL;
			for (int i = 0; i < 64; i++)
				this->Photo[i] = NULL;
			this->ReplyMarkup = NULL;
			NumPhoto = 0;
		}
		void FromJSON(json_t * ObjectEntry)
		{
			if (!ObjectEntry) return;
			json_t* JObj;
			this->FreeAll();
			FROM_JSON_GET_VALUE("message_id", this->Message_Id, json_integer_value);
			FROM_JSON_GET_OBJECT("from", this->From, TGBOT_User);
			FROM_JSON_GET_VALUE("date", this->Date, json_integer_value);
			FROM_JSON_GET_OBJECT("chat", this->Chat, TGBOT_Chat);
			FROM_JSON_GET_OBJECT("forward_from", this->ForwardFrom, TGBOT_User);
			FROM_JSON_GET_OBJECT("forward_from_chat", this->ForwardFromChat, TGBOT_Chat);
			FROM_JSON_GET_VALUE("forward_from_message_id", this->ForwardFromMessageId, json_integer_value);
			FROM_JSON_GET_VALUE("forward_signature", this->ForwardSignature, json_string_value);
			FROM_JSON_GET_VALUE("forward_sender_name", this->ForwardSenderName, json_string_value);
			FROM_JSON_GET_VALUE("forward_date", this->ForwardDate, json_integer_value);
			FROM_JSON_GET_OBJECT("reply_to_message", this->ReplyToMessage, TGBOT_Message);
			FROM_JSON_GET_VALUE("edit_date", this->EditDate, json_integer_value);
			FROM_JSON_GET_VALUE("media_group_id", this->MediaGroupId, json_string_value);
			FROM_JSON_GET_VALUE("author_signature", this->AuthorSignature, json_string_value);
			FROM_JSON_GET_VALUE("text", this->Text, json_string_value);
			FROM_JSON_GET_OBJECT("contact", this->Contact, TGBOT_Contact);
			FROM_JSON_GET_OBJECT("sticker", this->Sticker, TGBOT_Sticker);
			FROM_JSON_GET_OBJECT_ARRAY("photo", this->Photo, this->NumPhoto, TGBOT_PhotoSize);
			FROM_JSON_GET_OBJECT("reply_markup", this->ReplyMarkup, TGBOT_InlineKeyboardMarkup);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("message_id", this->Message_Id, true);
			TO_JSON_SET_OBJECT("from", this->From);
			TO_JSON_SET_VALUE("date", this->Date, true);
			TO_JSON_SET_OBJECT("chat", this->Chat);
			TO_JSON_SET_OBJECT("forward_from", this->ForwardFrom);
			TO_JSON_SET_OBJECT("forward_from_chat", this->ForwardFromChat);
			TO_JSON_SET_VALUE("forward_from_message_id", this->ForwardFromMessageId, true);
			TO_JSON_SET_VALUE("forward_signature", this->ForwardSignature, false);
			TO_JSON_SET_VALUE("forward_sender_name", this->ForwardSenderName, false);
			TO_JSON_SET_VALUE("forward_date", this->ForwardDate, true);
			TO_JSON_SET_OBJECT("reply_to_message", this->ReplyToMessage);
			TO_JSON_SET_VALUE("edit_date", this->EditDate, true);
			TO_JSON_SET_VALUE("media_group_id", this->MediaGroupId, false);
			TO_JSON_SET_VALUE("author_signature", this->AuthorSignature, false);
			TO_JSON_SET_VALUE("text", this->Text, false);
			TO_JSON_SET_OBJECT("contact", this->Contact);
			TO_JSON_SET_OBJECT("sticker", this->Sticker);
			TO_JSON_SET_OBJECT_ARRAY("photo", this->Photo, this->NumPhoto);
			TO_JSON_SET_OBJECT("reply_markup", this->ReplyMarkup);
			TO_JSON_SET_END;
		}
};

class TGBOT_CallbackQuery
{
	public:
		SMAnsiString  Id;
		TGBOT_User    *From;
		TGBOT_Message *Message;
		SMAnsiString  InlineMessageid,
		              ChatInstance,
		              Data,
		              GameShortName;

		TGBOT_CallbackQuery() { this->InitAll(); }
		TGBOT_CallbackQuery(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
		~TGBOT_CallbackQuery() { this->FreeAll(); }

		void FreeAll()
		{
			DELETE_SINGLE_OBJECT(this->From);
			DELETE_SINGLE_OBJECT(this->Message);
		}
		void InitAll()
		{
			this->From = NULL;
			this->Message = NULL;
		}

		void FromJSON(json_t* ObjectEntry)
		{
			if (!ObjectEntry) return;
			json_t* JObj;
			this->FreeAll();
			FROM_JSON_GET_VALUE("id", this->Id, json_string_value);
			FROM_JSON_GET_OBJECT("from", this->From, TGBOT_User);
			FROM_JSON_GET_OBJECT("message", this->Message, TGBOT_Message);
			FROM_JSON_GET_VALUE("inline_message_id", this->InlineMessageid, json_string_value);
			FROM_JSON_GET_VALUE("chat_instance", this->ChatInstance, json_string_value);
			FROM_JSON_GET_VALUE("data", this->Data, json_string_value);
			FROM_JSON_GET_VALUE("game_short_name", this->GameShortName, json_string_value);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("message_id", this->Id, false);
			TO_JSON_SET_OBJECT("from", this->From);
			TO_JSON_SET_OBJECT("chat", this->Message);
			TO_JSON_SET_VALUE("forward_from_message_id", this->InlineMessageid, false);
			TO_JSON_SET_VALUE("forward_signature", this->ChatInstance, false);
			TO_JSON_SET_VALUE("forward_sender_name", this->Data, false);
			TO_JSON_SET_VALUE("forward_date", this->GameShortName, false);
			TO_JSON_SET_END;
		}
};

class TGBOT_Update
{
	public:
		uint64_t            UpdateId;
		TGBOT_Message       *Message,
		                    *EditedMessage,
		                    *ChannelPost,
		                    *EditedChannelPost;
		TGBOT_CallbackQuery *CallbackQuery;

		TGBOT_Update() { this->InitAll(); }
		TGBOT_Update(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
		~TGBOT_Update() { this->FreeAll(); }

		void FreeAll()
		{
			DELETE_SINGLE_OBJECT(this->Message);
			DELETE_SINGLE_OBJECT(this->EditedMessage);
			DELETE_SINGLE_OBJECT(this->ChannelPost);
			DELETE_SINGLE_OBJECT(this->EditedChannelPost);
			DELETE_SINGLE_OBJECT(this->CallbackQuery);
		}
		void InitAll()
		{
			this->UpdateId = 0;
			this->Message = NULL;
			this->EditedMessage = NULL;
			this->ChannelPost = NULL;
			this->EditedChannelPost = NULL;
			this->CallbackQuery = NULL;
		}

		void FromJSON(json_t* ObjectEntry)
		{
			if (!ObjectEntry) return;
			json_t* JObj;
			this->FreeAll();
			FROM_JSON_GET_VALUE("update_id", this->UpdateId, json_integer_value);
			FROM_JSON_GET_OBJECT("message", this->Message, TGBOT_Message);
			FROM_JSON_GET_OBJECT("edited_message", this->EditedMessage, TGBOT_Message);
			FROM_JSON_GET_OBJECT("channel_post", this->ChannelPost, TGBOT_Message);
			FROM_JSON_GET_OBJECT("edited_channel_post", this->EditedChannelPost, TGBOT_Message);
			FROM_JSON_GET_OBJECT("callback_query", this->CallbackQuery, TGBOT_CallbackQuery);
		}
		SMAnsiString ToJSON()
		{
			TO_JSON_SET_START;
			TO_JSON_SET_VALUE("update_id", this->UpdateId, true);
			TO_JSON_SET_OBJECT("message", this->Message);
			TO_JSON_SET_OBJECT("edited_message", this->EditedMessage);
			TO_JSON_SET_OBJECT("channel_post", this->ChannelPost);
			TO_JSON_SET_OBJECT("edited_channel_post", this->EditedChannelPost);
			TO_JSON_SET_OBJECT("callback_query", this->CallbackQuery);
			TO_JSON_SET_END;
		}
};

bool tgbot_GetUpdates(std::vector<TGBOT_Update*>& updates, uint64_t offset = 0);
uint64_t GetChatIDFromUpdate(TGBOT_Update* Upd);
void tgbot_answerCallbackQuery(SMAnsiString CallbackQueryID);
TGBOT_User* tgbot_getMe();
void tgbot_SendMessage(uint64_t ChatID, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard = nullptr);
void tgbot_SendMessage(SMKeyList* ChatIDList, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard = nullptr);
void tgbot_forwardMessage(uint64_t ChatID, uint64_t FromChatID, uint64_t MessageID);
void tgbot_SendPhotoWithUpload(SMAnsiString Filename, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_SendPhotoByFileId(const SMAnsiString& FileID, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_SendDocument(const char* buf, size_t buflen, const SMAnsiString& outFileName, const SMAnsiString& contentType, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_editMessageText(uint64_t chat_id, uint64_t message_id, const SMAnsiString& text, TGBOT_InlineKeyboardMarkup* keyboard=nullptr);
void tgbot_sendVenue(uint64_t chat_id, double latitude, double longitude, const SMAnsiString& title, const SMAnsiString& address, TGBOT_InlineKeyboardMarkup* keyboard=nullptr);
void tgbot_deleteMessage(uint64_t chat_id, uint64_t message_id);
void tgbot_sendSticker(uint64_t chat_id, const SMAnsiString &sticker);

#endif