#ifndef TGBOT_BOTAPI_HEADER
#define TGBOT_BOTAPI_HEADER

#include "tg_bot.h"

// массив массивов с объектами
template <class T>
class TGBOT_ARRAY_OF_ARRAY: public std::vector<std::vector<std::unique_ptr<T>>>
{
	using inherited = std::vector<std::vector<std::unique_ptr<T>>>;
	using uptr      = std::unique_ptr<T>;

	private:
		size_t current_row;

		inline void placeRow()
		{
			std::vector<uptr> row;
			inherited::push_back(std::move(row));
		}
		inline void copy(const TGBOT_ARRAY_OF_ARRAY<T>& arr)
		{
			inherited::clear();
			current_row = arr.current_row;
			const size_t numrow = arr.size();
			for (size_t i = 0; i < numrow; i++)
			{
				placeRow();
				const size_t numcol = arr.at(i).size();
				for (size_t j = 0; j < numcol; j++)
				{
					uptr obj(new T(arr.at(i).at(j).get()));
					this->at(i).push_back(std::move(obj));
				}
			}
		}
	public:
		size_t GetCurrentRow() { return current_row; }

		TGBOT_ARRAY_OF_ARRAY(): current_row(0ull) { placeRow(); }
		TGBOT_ARRAY_OF_ARRAY(const TGBOT_ARRAY_OF_ARRAY<T> &arr) { copy(arr); }

		size_t CreateRow() { placeRow(); return ++current_row; }
		void push_back(T* val)
		{
			uptr obj(val);
			this->at(current_row).push_back(std::move(obj));
		}
		void clear()
		{
			inherited::clear();
			placeRow();
			current_row = 0ull;
		}

		TGBOT_ARRAY_OF_ARRAY<T>& operator=(const TGBOT_ARRAY_OF_ARRAY<T>& val)
		{
			copy(val);
			return *this;
		}
};
// массив с объектами
#define TGBOT_ARRAY(type) std::vector<std::unique_ptr<type>>

template <typename T>
inline void GetArrayOfArrayFromJSON(json_t* JEntry, const char* json_fld, TGBOT_ARRAY_OF_ARRAY<T> &struct_arr_of_arr)
{
	json_t *JObj;
	json_t *JArr2;
	json_t *JArr1 = json_object_get(JEntry, json_fld);
	if (JArr1 != nullptr)
	{
		size_t numrows = json_array_size(JArr1);
		for (size_t i = 0; i < numrows; i++)
		{
			JArr2 = json_array_get(JArr1, i);
			size_t numbtn = json_array_size(JArr2);
			for (size_t j = 0; j < numbtn; j++)
			{
				JObj = json_array_get(JArr2, j);
				T *obj = new T(JObj);
				struct_arr_of_arr.push_back(obj);
			}
		}
	}
}

template <typename T>
inline void GetArrayFromJSON(json_t* JEntry, const char* json_fld, TGBOT_ARRAY(T) &struct_arr)
{
	json_t* JObj = nullptr;
	json_t* JArr = json_object_get(JEntry, json_fld);
	if (JArr != nullptr)
	{
		size_t numelem = json_array_size(JArr);
		for (size_t i = 0; i < numelem; i++)
		{
			JObj = json_array_get(JArr, i);
			std::unique_ptr<T> uptr(new T(JObj));
			struct_arr.push_back(std::move(uptr));
		}
	}
}

template <typename IntType>
static inline IntType jsonint_to_int(const json_t* json) { return static_cast<IntType>(json_integer_value(json)); }

template <typename T>
inline void GetValueFromJSON(json_t* JEntry, const char* json_fld, T* &struct_fld)
{
	json_t* JObj = json_object_get(JEntry, json_fld);
	if (JObj != nullptr)
	{
		struct_fld = new T(JObj);
	}
}

#define GetStdValueFromJSON(type, json_type_func)                                          \
inline void GetValueFromJSON(json_t* JEntry, const char *json_fld, type &struct_fld)       \
{                                                                                          \
	json_t* JObj = json_object_get(JEntry, json_fld);                                      \
	if (JObj != NULL)                                                                      \
	{                                                                                      \
		struct_fld = json_type_func(JObj);                                                 \
	}                                                                                      \
}
GetStdValueFromJSON(long long, json_integer_value);
GetStdValueFromJSON(int, jsonint_to_int<int>);
GetStdValueFromJSON(uint64_t, jsonint_to_int<uint64_t>);
GetStdValueFromJSON(SMAnsiString, json_string_value);
GetStdValueFromJSON(double, json_real_value);
GetStdValueFromJSON(bool, json_boolean_value);
#ifdef __GNUG__
GetStdValueFromJSON(time_t, jsonint_to_int<time_t>);
#endif

static inline void DoStartStream(SMAnsiString& s)
{
	if (s.IsEmpty()) s = '{';
	else s[s.length() - 1] = '\0';
	if (s.length() > 1) s += ',';
}

template <typename T>
inline void PutArrayOfArrayToJSON(SMAnsiString& s, const char* json_fld, TGBOT_ARRAY_OF_ARRAY<T> &struct_arr_of_arr)
{
	if (IsStrEmpty(json_fld)) return;
	DoStartStream(s);
	s += SMAnsiString::smprintf("\"%s\":[", json_fld);
	const size_t numrow = struct_arr_of_arr.size();
	for (size_t i = 0; i < numrow; i++)
	{
		const size_t numcol = struct_arr_of_arr.at(i).size();
		if (!numcol) break;
		if (i) s += ',';
		s += '[';
		for (size_t j = 0; j < numcol; j++)
		{
			s += struct_arr_of_arr.at(i).at(j)->ToJSON();
		}
		s += ']';
	}
	s += "]}";
}

template <typename T>
inline void PutArrayToJSON(SMAnsiString &s, const char* json_fld, TGBOT_ARRAY(T) &struct_arr)
{
	if (IsStrEmpty(json_fld)) return;
	DoStartStream(s);
	s += SMAnsiString::smprintf("\"%s\":[", json_fld);
	const size_t numrow = struct_arr.size();
	for (size_t i = 0; i < numrow; i++)
	{
		s += struct_arr.at(i)->ToJSON();
	}
	s += "]}";
}

template <typename T>
inline void PutValueToJSON(SMAnsiString &s, const char *json_fld, T *struct_fld)
{
	if (!struct_fld || IsStrEmpty(json_fld)) return;
	DoStartStream(s);
	s += SMAnsiString::smprintf("\"%s\":%s}", json_fld, C_STR(struct_fld->ToJSON()));
}

#define PutStdValueToJSON(type, is_integer)                                                        \
inline void PutValueToJSON(SMAnsiString &s, const char *json_fld, type &struct_fld)                \
{                                                                                                  \
	if (!struct_fld || IsStrEmpty(json_fld)) return;                                               \
	if (!static_cast<bool>(struct_fld)) return;                                                    \
	DoStartStream(s);                                                                              \
	if(is_integer)                                                                                 \
		s += SMAnsiString::smprintf("\"%s\":%s}", json_fld, C_STR(SMAnsiString(struct_fld)));      \
	else                                                                                           \
		s += SMAnsiString::smprintf("\"%s\":\"%s\"}", json_fld, C_STR(SMAnsiString(struct_fld)));  \
}
PutStdValueToJSON(long long,    true );
PutStdValueToJSON(int,          true );
PutStdValueToJSON(uint64_t,     true );
PutStdValueToJSON(SMAnsiString, false);
PutStdValueToJSON(double,       true );
PutStdValueToJSON(bool,         true );
#ifdef __GNUG__
PutStdValueToJSON(time_t,        true);
#endif

// Основные примитивы Telegram API
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
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "width",          Width       );
			GetValueFromJSON(ObjectEntry, "height",         Height      );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "width",          Width       );
			PutValueToJSON(ostream, "height",         Height      );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			return std::move(ostream);
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
			GetValueFromJSON(ObjectEntry, "point",   Point );
			GetValueFromJSON(ObjectEntry, "x_shift", XShift);
			GetValueFromJSON(ObjectEntry, "y_shift", YShift);
			GetValueFromJSON(ObjectEntry, "scale",   Scale );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "point",   Point );
			PutValueToJSON(ostream, "x_shift", XShift);
			PutValueToJSON(ostream, "y_shift", YShift);
			PutValueToJSON(ostream, "scale",   Scale );
			return std::move(ostream);
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
			GetValueFromJSON(ObjectEntry, "id",            Id          );
			GetValueFromJSON(ObjectEntry, "is_bot",        Is_Bot      );
			GetValueFromJSON(ObjectEntry, "first_name",    FirstName   );
			GetValueFromJSON(ObjectEntry, "last_name",     LastName    );
			GetValueFromJSON(ObjectEntry, "username",      Username    );
			GetValueFromJSON(ObjectEntry, "language_code", LanguageCode);
			GetValueFromJSON(ObjectEntry, "is_premium",    IsPremium   );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",            Id          );
			PutValueToJSON(ostream, "is_bot",        Is_Bot      );
			PutValueToJSON(ostream, "first_name",    FirstName   );
			PutValueToJSON(ostream, "last_name",     LastName    );
			PutValueToJSON(ostream, "username",      Username    );
			PutValueToJSON(ostream, "language_code", LanguageCode);
			PutValueToJSON(ostream, "is_premium",    IsPremium   );
			return std::move(ostream);
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
			this->FreeAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId);
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "width",          Width);
			GetValueFromJSON(ObjectEntry, "height",         Height);
			GetValueFromJSON(ObjectEntry, "is_animated",    IsAnimated);
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb);
			GetValueFromJSON(ObjectEntry, "emoji",          Emoji);
			GetValueFromJSON(ObjectEntry, "set_name",       SetName);
			GetValueFromJSON(ObjectEntry, "mask_position",  MaskPosition);
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize);
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId);
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "width",          Width);
			PutValueToJSON(ostream, "height",         Height);
			PutValueToJSON(ostream, "is_animated",    IsAnimated);
			PutValueToJSON(ostream, "thumb",          Thumb);
			PutValueToJSON(ostream, "emoji",          Emoji);
			PutValueToJSON(ostream, "set_name",       SetName);
			PutValueToJSON(ostream, "mask_position",  MaskPosition);
			PutValueToJSON(ostream, "file_size",      FileSize);
			return std::move(ostream);
		}
};

class TGBOT_StickerSet
{
	public:
		SMAnsiString               Name,
					               Title;
		bool                       IsAnimated,
					               ContainsMasks;
		TGBOT_ARRAY(TGBOT_Sticker) Stickers;

		TGBOT_StickerSet() {}
		TGBOT_StickerSet(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void FreeAll()
		{
			Stickers.clear();
		}
		void FromJSON(json_t * ObjectEntry)
		{
			this->FreeAll();
			GetValueFromJSON(ObjectEntry, "name",           Name          );
			GetValueFromJSON(ObjectEntry, "title",          Title         );
			GetValueFromJSON(ObjectEntry, "is_animated",    IsAnimated    );
			GetValueFromJSON(ObjectEntry, "contains_masks", ContainsMasks );
			GetArrayFromJSON(ObjectEntry, "stickers",       Stickers      );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "name",           Name          );
			PutValueToJSON(ostream, "title",          Title         );
			PutValueToJSON(ostream, "is_animated",    IsAnimated    );
			PutValueToJSON(ostream, "contains_masks", ContainsMasks );
			PutArrayToJSON(ostream, "stickers",       Stickers      );
			return std::move(ostream);
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
			GetValueFromJSON(ObjectEntry, "id",         Id       );
			GetValueFromJSON(ObjectEntry, "type",       Type     );
			GetValueFromJSON(ObjectEntry, "title",      Title    );
			GetValueFromJSON(ObjectEntry, "first_name", Username );
			GetValueFromJSON(ObjectEntry, "last_name",  FirstName);
			GetValueFromJSON(ObjectEntry, "username",   LastName );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",         Id       );
			PutValueToJSON(ostream, "type",       Type     );
			PutValueToJSON(ostream, "title",      Title    );
			PutValueToJSON(ostream, "first_name", Username );
			PutValueToJSON(ostream, "last_name",  FirstName);
			PutValueToJSON(ostream, "username",   LastName );
			return std::move(ostream);
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
			GetValueFromJSON(ObjectEntry, "phone_number",  PhoneNumber);
			GetValueFromJSON(ObjectEntry, "first_name",    FirstName  );
			GetValueFromJSON(ObjectEntry, "last_name",     LastName   );
			GetValueFromJSON(ObjectEntry, "user_id",       UserId     );
			GetValueFromJSON(ObjectEntry, "vcard",         Vcard      );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "phone_number",  PhoneNumber);
			PutValueToJSON(ostream, "first_name",    FirstName  );
			PutValueToJSON(ostream, "last_name",     LastName   );
			PutValueToJSON(ostream, "user_id",       UserId     );
			PutValueToJSON(ostream, "vcard",         Vcard      );
			return std::move(ostream);
		}
};

class TGBOT_KeyboardButton
{
	public:
		SMAnsiString Text;
		bool         RequestContact,
		             RequestLocation;

		TGBOT_KeyboardButton() {}
		TGBOT_KeyboardButton(const TGBOT_KeyboardButton &val);
		TGBOT_KeyboardButton(const TGBOT_KeyboardButton *val);
		TGBOT_KeyboardButton(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }
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
			GetValueFromJSON(ObjectEntry, "text",             Text           );
			GetValueFromJSON(ObjectEntry, "request_contact",  RequestContact );
			GetValueFromJSON(ObjectEntry, "request_location", RequestLocation);
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "text",             Text           );
			PutValueToJSON(ostream, "request_contact",  RequestContact );
			PutValueToJSON(ostream, "request_location", RequestLocation);
			return std::move(ostream);
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
		TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton *b);
		TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton &b);
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
			GetValueFromJSON(ObjectEntry, "text",                             Text                        );
			GetValueFromJSON(ObjectEntry, "url",                              Url                         );
			GetValueFromJSON(ObjectEntry, "callback_data",                    CallbackData                );
			GetValueFromJSON(ObjectEntry, "switch_inline_query",              SwitchInlineQuery           );
			GetValueFromJSON(ObjectEntry, "switch_inline_query_current_chat", SwitchInlineQueryCurrentChat);
			GetValueFromJSON(ObjectEntry, "pay",                              Pay                         );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "text",                             Text                        );
			PutValueToJSON(ostream, "url",                              Url                         );
			PutValueToJSON(ostream, "callback_data",                    CallbackData                );
			PutValueToJSON(ostream, "switch_inline_query",              SwitchInlineQuery           );
			PutValueToJSON(ostream, "switch_inline_query_current_chat", SwitchInlineQueryCurrentChat);
			PutValueToJSON(ostream, "pay",                              Pay                         );
			return std::move(ostream);
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
		TGBOT_ARRAY_OF_ARRAY<TGBOT_KeyboardButton> Keyboard;
		bool                                       ResizeKeyboard,
		                                           OneTimeKeyboard,
		                                           Selective;

	public:
		TGBOT_ReplyKeyboardMarkup() { Init(true, false, false, false); }
		TGBOT_ReplyKeyboardMarkup(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective) { Init(ResizeKeyboard, OneTimeKeyboard, Selective, false); }

		void Init(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective, bool freemem=true)
		{
			Keyboard.clear();
			this->ResizeKeyboard = ResizeKeyboard;
			this->OneTimeKeyboard = OneTimeKeyboard;
			this->Selective = Selective;
		}

		void CreateButton(SMAnsiString Text)
		{
			Keyboard.push_back(new TGBOT_KeyboardButton(Text, false, false));
		}
		void CreateRow() { Keyboard.CreateRow(); }
	
		virtual void FromJSON(json_t* ObjectEntry)
		{
			GetValueFromJSON       (ObjectEntry, "resize_keyboard",   ResizeKeyboard );
			GetValueFromJSON       (ObjectEntry, "one_time_keyboard", OneTimeKeyboard);
			GetValueFromJSON       (ObjectEntry, "selective",         Selective      );
			GetArrayOfArrayFromJSON(ObjectEntry, "keyboard",          Keyboard       );
		}
		virtual SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON       (ostream, "resize_keyboard",   ResizeKeyboard );
			PutValueToJSON       (ostream, "one_time_keyboard", OneTimeKeyboard);
			PutValueToJSON       (ostream, "selective",         Selective      );
			PutArrayOfArrayToJSON(ostream, "keyboard",          Keyboard       );
			return std::move(ostream);
		}
};

class TGBOT_InlineKeyboardMarkup: public TGBOT_Keyboard
{
	public:
		TGBOT_ARRAY_OF_ARRAY<TGBOT_InlineKeyboardButton> InlineKeyboard;

		TGBOT_InlineKeyboardMarkup() {}
		TGBOT_InlineKeyboardMarkup(const TGBOT_InlineKeyboardMarkup& kb) { InlineKeyboard = kb.InlineKeyboard; }
		TGBOT_InlineKeyboardMarkup(json_t* ObjectEntry) { this->FromJSON(ObjectEntry); }

		void Init() { InlineKeyboard.clear(); }

		void CreateButton(SMAnsiString Text)
		{
			InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text));
		}
		void CreateButton(SMAnsiString Text, SMAnsiString Url)
		{
			InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text, Url));
		}
		void CreateButton(SMAnsiString Text, SMAnsiString CallbackData, bool Pay)
		{
			InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text, CallbackData, Pay));
		}
		void CreateRow() { InlineKeyboard.CreateRow(); }

		virtual void FromJSON(json_t * ObjectEntry)
		{
			GetArrayOfArrayFromJSON(ObjectEntry, "inline_keyboard", InlineKeyboard);
		}
		virtual SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutArrayOfArrayToJSON(ostream, "inline_keyboard", InlineKeyboard);
			return std::move(ostream);
		}
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
			GetValueFromJSON(ObjectEntry, "force_reply", ForceReply);
			GetValueFromJSON(ObjectEntry, "selective",   Selective );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "force_reply", ForceReply);
			PutValueToJSON(ostream, "selective",   Selective );
			return std::move(ostream);
		}
};

class TGBOT_Message
{
	public:
		uint64_t                     Message_Id;
		uint64_t                     MessageThreadId;
		TGBOT_User*                  From;
		time_t                       Date;
		TGBOT_Chat*                  Chat;
		TGBOT_User*                  ForwardFrom;
		TGBOT_Chat*                  ForwardFromChat;
		uint64_t                     ForwardFromMessageId;
		SMAnsiString                 ForwardSignature;
		SMAnsiString                 ForwardSenderName;
		time_t                       ForwardDate;
		bool                         IsTopicMessage;
		bool                         IsAutomaticForward;
		TGBOT_Message*               ReplyToMessage;
		time_t                       EditDate;
		bool                         HasProtectedContent;
		SMAnsiString                 MediaGroupId;
		SMAnsiString                 AuthorSignature;
		SMAnsiString                 Text;
		TGBOT_ARRAY(TGBOT_PhotoSize) Photo;
		TGBOT_Sticker*               Sticker;
		SMAnsiString                 Caption;
		TGBOT_Contact*               Contact;
		SMAnsiString                 NewChatTitle;
		bool                         DeleteChatPhoto;
		bool                         GroupChatCreated;
		bool                         SupergroupChatCreated;
		bool                         ChannelChatCreated;
		TGBOT_InlineKeyboardMarkup*  ReplyMarkup;

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
			DELETE_SINGLE_OBJECT(this->ReplyMarkup);
		}
		void InitAll()
		{
			IsTopicMessage = false;
			IsAutomaticForward = false;
			HasProtectedContent = false;
			DeleteChatPhoto = false;
			GroupChatCreated = false;
			SupergroupChatCreated = false;
			ChannelChatCreated = false;

			Photo.clear();
			this->From = NULL;
			this->Chat = NULL;
			this->ForwardFrom = NULL;
			this->ForwardFromChat = NULL;
			this->ReplyToMessage = NULL;
			this->Contact = NULL;
			this->Sticker = NULL;
			this->ReplyMarkup = NULL;
		}
		void FromJSON(json_t *ObjectEntry)
		{
			this->FreeAll();
			GetValueFromJSON(ObjectEntry, "message_id",              Message_Id           );
			GetValueFromJSON(ObjectEntry, "message_thread_id",       MessageThreadId      );
			GetValueFromJSON(ObjectEntry, "from",                    From                  );
			GetValueFromJSON(ObjectEntry, "date",                    Date                 );
			GetValueFromJSON(ObjectEntry, "chat",                    Chat                  );
			GetValueFromJSON(ObjectEntry, "forward_from",            ForwardFrom           );
			GetValueFromJSON(ObjectEntry, "forward_from_chat",       ForwardFromChat       );
			GetValueFromJSON(ObjectEntry, "forward_from_message_id", ForwardFromMessageId );
			GetValueFromJSON(ObjectEntry, "forward_signature",       ForwardSignature     );
			GetValueFromJSON(ObjectEntry, "forward_sender_name",     ForwardSenderName    );
			GetValueFromJSON(ObjectEntry, "forward_date",            ForwardDate          );
			GetValueFromJSON(ObjectEntry, "is_topic_message",        IsTopicMessage       );
			GetValueFromJSON(ObjectEntry, "is_automatic_forward",    IsAutomaticForward   );
			GetValueFromJSON(ObjectEntry, "reply_to_message",        ReplyToMessage        );
			GetValueFromJSON(ObjectEntry, "edit_date",               EditDate             );
			GetValueFromJSON(ObjectEntry, "has_protected_content",   HasProtectedContent  );
			GetValueFromJSON(ObjectEntry, "media_group_id",          MediaGroupId         );
			GetValueFromJSON(ObjectEntry, "author_signature",        AuthorSignature      );
			GetValueFromJSON(ObjectEntry, "text",                    Text                 );
			GetArrayFromJSON(ObjectEntry, "photo",                   Photo                );
			GetValueFromJSON(ObjectEntry, "sticker",                 Sticker               );
			GetValueFromJSON(ObjectEntry, "caption",                 Caption              );
			GetValueFromJSON(ObjectEntry, "contact",                 Contact               );
			GetValueFromJSON(ObjectEntry, "new_chat_title",          NewChatTitle         );
			GetValueFromJSON(ObjectEntry, "delete_chat_photo",       DeleteChatPhoto      );
			GetValueFromJSON(ObjectEntry, "group_chat_created",      GroupChatCreated     );
			GetValueFromJSON(ObjectEntry, "supergroup_chat_created", SupergroupChatCreated);
			GetValueFromJSON(ObjectEntry, "channel_chat_created",    ChannelChatCreated   );
			GetValueFromJSON(ObjectEntry, "reply_markup",            ReplyMarkup           );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "message_id",              Message_Id           );
			PutValueToJSON(ostream, "message_thread_id",       MessageThreadId      );
			PutValueToJSON(ostream, "from",                    From                  );
			PutValueToJSON(ostream, "date",                    Date                 );
			PutValueToJSON(ostream, "chat",                    Chat                  );
			PutValueToJSON(ostream, "forward_from",            ForwardFrom           );
			PutValueToJSON(ostream, "forward_from_chat",       ForwardFromChat       );
			PutValueToJSON(ostream, "forward_from_message_id", ForwardFromMessageId );
			PutValueToJSON(ostream, "forward_signature",       ForwardSignature     );
			PutValueToJSON(ostream, "forward_sender_name",     ForwardSenderName    );
			PutValueToJSON(ostream, "forward_date",            ForwardDate          );
			PutValueToJSON(ostream, "is_topic_message",        IsTopicMessage       );
			PutValueToJSON(ostream, "is_automatic_forward",    IsAutomaticForward   );
			PutValueToJSON(ostream, "reply_to_message",        ReplyToMessage        );
			PutValueToJSON(ostream, "edit_date",               EditDate             );
			PutValueToJSON(ostream, "has_protected_content",   HasProtectedContent  );
			PutValueToJSON(ostream, "media_group_id",          MediaGroupId         );
			PutValueToJSON(ostream, "author_signature",        AuthorSignature      );
			PutValueToJSON(ostream, "text",                    Text                 );
			PutArrayToJSON(ostream, "photo",                   Photo                );
			PutValueToJSON(ostream, "sticker",                 Sticker               );
			PutValueToJSON(ostream, "caption",                 Caption              );
			PutValueToJSON(ostream, "contact",                 Contact               );
			PutValueToJSON(ostream, "new_chat_title",          NewChatTitle         );
			PutValueToJSON(ostream, "delete_chat_photo",       DeleteChatPhoto      );
			PutValueToJSON(ostream, "group_chat_created",      GroupChatCreated     );
			PutValueToJSON(ostream, "supergroup_chat_created", SupergroupChatCreated);
			PutValueToJSON(ostream, "channel_chat_created",    ChannelChatCreated   );
			PutValueToJSON(ostream, "reply_markup",            ReplyMarkup           );
			return std::move(ostream);
		}
};

class TGBOT_CallbackQuery
{
	public:
		SMAnsiString  Id;
		TGBOT_User    *From;
		TGBOT_Message *Message;
		SMAnsiString  InlineMessageId,
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
			this->FreeAll();
			GetValueFromJSON(ObjectEntry, "id",                Id);
			GetValueFromJSON(ObjectEntry, "from",              From);
			GetValueFromJSON(ObjectEntry, "message",           Message);
			GetValueFromJSON(ObjectEntry, "inline_message_id", InlineMessageId);
			GetValueFromJSON(ObjectEntry, "chat_instance",     ChatInstance);
			GetValueFromJSON(ObjectEntry, "data",              Data);
			GetValueFromJSON(ObjectEntry, "game_short_name",   GameShortName);
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",                Id             );
			PutValueToJSON(ostream, "from",              From            );
			PutValueToJSON(ostream, "message",           Message         );
			PutValueToJSON(ostream, "inline_message_id", InlineMessageId);
			PutValueToJSON(ostream, "chat_instance",     ChatInstance   );
			PutValueToJSON(ostream, "data",              Data           );
			PutValueToJSON(ostream, "game_short_name",   GameShortName  );
			return std::move(ostream);
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
			this->FreeAll();
			GetValueFromJSON(ObjectEntry, "update_id",           UpdateId        );
			GetValueFromJSON(ObjectEntry, "message",             Message          );
			GetValueFromJSON(ObjectEntry, "edited_message",      EditedMessage    );
			GetValueFromJSON(ObjectEntry, "channel_post",        ChannelPost      );
			GetValueFromJSON(ObjectEntry, "edited_channel_post", EditedChannelPost);
			GetValueFromJSON(ObjectEntry, "callback_query",      CallbackQuery    );
		}
		SMAnsiString ToJSON()
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "update_id",           UpdateId        );
			PutValueToJSON(ostream, "message",             Message          );
			PutValueToJSON(ostream, "edited_message",      EditedMessage    );
			PutValueToJSON(ostream, "channel_post",        ChannelPost      );
			PutValueToJSON(ostream, "edited_channel_post", EditedChannelPost);
			PutValueToJSON(ostream, "callback_query",      CallbackQuery    );
			return std::move(ostream);
		}
};

bool tgbot_GetUpdates(std::vector<TGBOT_Update*>& updates, uint64_t offset = 0);
uint64_t GetChatIDFromUpdate(TGBOT_Update* Upd);
void tgbot_answerCallbackQuery(const SMAnsiString &CallbackQueryID);
TGBOT_User* tgbot_getMe();
void tgbot_SendMessage(uint64_t ChatID, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard = nullptr);
void tgbot_SendMessage(SMKeyList* ChatIDList, const SMAnsiString &Text, TGBOT_Keyboard *Keyboard = nullptr);
void tgbot_forwardMessage(uint64_t ChatID, uint64_t FromChatID, uint64_t MessageID);
void tgbot_SendPhotoWithUpload(const SMAnsiString &Filename, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_SendPhotoByFileId(const SMAnsiString& FileID, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_SendDocument(const char* buf, size_t buflen, const SMAnsiString& outFileName, const SMAnsiString& contentType, uint64_t ChatID, const SMAnsiString& Caption);
void tgbot_editMessageText(uint64_t chat_id, uint64_t message_id, const SMAnsiString& text, TGBOT_InlineKeyboardMarkup* keyboard=nullptr);
void tgbot_sendVenue(uint64_t chat_id, double latitude, double longitude, const SMAnsiString& title, const SMAnsiString& address, TGBOT_InlineKeyboardMarkup* keyboard=nullptr);
void tgbot_deleteMessage(uint64_t chat_id, uint64_t message_id);
void tgbot_sendSticker(uint64_t chat_id, const SMAnsiString &sticker);

#endif