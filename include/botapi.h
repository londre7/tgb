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
		size_t current_row = 0ull;

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

		TGBOT_ARRAY_OF_ARRAY() { placeRow(); }
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

#define GetStdArrayFromJSON(type, json_type_func) \
template<>                                                                                                \
inline void GetArrayFromJSON(json_t* JEntry, const char* json_fld, TGBOT_ARRAY(type) &struct_arr)         \
{                                                                                                         \
	json_t* JObj = nullptr;                                                                               \
	json_t* JArr = json_object_get(JEntry, json_fld);                                                     \
	if (JArr != nullptr)                                                                                  \
	{                                                                                                     \
		size_t numelem = json_array_size(JArr);                                                           \
		for (size_t i = 0; i < numelem; i++)                                                              \
		{                                                                                                 \
			JObj = json_array_get(JArr, i);                                                               \
			std::unique_ptr<type> uptr(new type(json_type_func(JObj)));                                   \
			struct_arr.push_back(std::move(uptr));                                                        \
		}                                                                                                 \
	}                                                                                                     \
}
GetStdArrayFromJSON(long long,    json_integer_value      );
GetStdArrayFromJSON(int,          jsonint_to_int<int>     );
GetStdArrayFromJSON(uint64_t,     jsonint_to_int<uint64_t>);
GetStdArrayFromJSON(SMAnsiString, json_string_value       );
GetStdArrayFromJSON(double,       json_real_value         );
GetStdArrayFromJSON(bool,         json_boolean_value      );
#ifdef __GNUG__
GetStdArrayFromJSON(time_t,       jsonint_to_int<time_t>  );
#endif

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
GetStdValueFromJSON(long long,    json_integer_value      );
GetStdValueFromJSON(int,          jsonint_to_int<int>     );
GetStdValueFromJSON(uint64_t,     jsonint_to_int<uint64_t>);
GetStdValueFromJSON(SMAnsiString, json_string_value       );
GetStdValueFromJSON(double,       json_real_value         );
GetStdValueFromJSON(bool,         json_boolean_value      );
#ifdef __GNUG__
GetStdValueFromJSON(time_t,       jsonint_to_int<time_t>  );
#endif

static inline void DoStartStream(SMAnsiString& s)
{
	if (s.IsEmpty()) s = '{';
	else s[s.length() - 1ull] = '\0';
	if (s.length() > 1ull) s += ',';
}

template <typename T>
inline void PutArrayOfArrayToJSON(SMAnsiString& s, const char* json_fld, const TGBOT_ARRAY_OF_ARRAY<T> &struct_arr_of_arr)
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
inline void PutArrayToJSON(SMAnsiString &s, const char* json_fld, const TGBOT_ARRAY(T) &struct_arr)
{
	if (IsStrEmpty(json_fld)) return;
	DoStartStream(s);
	s += SMAnsiString::smprintf("\"%s\":[", json_fld);
	const size_t numrow = struct_arr.size();
	for (size_t i = 0; i < numrow; i++)
	{
		if (i) s += ',';
		s += struct_arr.at(i)->ToJSON();
	}
	s += "]}";
}

#define SetStdArrayToJSON(type, is_integer) \
template <>                                                                                      \
inline void PutArrayToJSON(SMAnsiString &s, const char* json_fld, const TGBOT_ARRAY(type) &struct_arr) \
{                                                                                                \
	if (IsStrEmpty(json_fld)) return;                                                            \
	DoStartStream(s);                                                                            \
	s += SMAnsiString::smprintf("\"%s\":[", json_fld);                                           \
	const size_t numrow = struct_arr.size();                                                     \
	for (size_t i = 0; i < numrow; i++)                                                          \
	{                                                                                            \
        const type *ptr = struct_arr.at(i).get();                                                \
		if (i) s += ',';                                                                         \
		if (!is_integer)                                                                         \
			s += SMAnsiString::smprintf("\"%s\"", C_STR(SMAnsiString(*ptr)));                    \
		else                                                                                     \
			s += SMAnsiString(*ptr);                                                             \
	}                                                                                            \
	s += "]}";                                                                                   \
}
SetStdArrayToJSON(long long,    true );
SetStdArrayToJSON(int,          true );
SetStdArrayToJSON(uint64_t,     true );
SetStdArrayToJSON(SMAnsiString, false);
SetStdArrayToJSON(double,       true );
SetStdArrayToJSON(bool,         true );
#ifdef __GNUG__
SetStdArrayToJSON(time_t, true);
#endif

template <typename T>
inline void PutValueToJSON(SMAnsiString &s, const char *json_fld, const T *struct_fld)
{
	if (!struct_fld || IsStrEmpty(json_fld)) return;
	DoStartStream(s);
	s += SMAnsiString::smprintf("\"%s\":%s}", json_fld, C_STR(struct_fld->ToJSON()));
}

#define PutStdValueToJSON(type, is_integer)                                                        \
inline void PutValueToJSON(SMAnsiString &s, const char *json_fld, const type &struct_fld)          \
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
PutStdValueToJSON(time_t,       true );
#endif

#define ADD_CONSTRUCTORS(clsname)                                              \
clsname() { this->InitAll(); }                                                 \
clsname(json_t* ObjectEntry) { this->InitAll(); this->FromJSON(ObjectEntry); }
#define ADD_DESTRUCTORS(clsname) ~clsname() { this->FreeAll(); }
#define ADD_EXTERNAL_CONSTRUCTORS(clsname) \
clsname();                                 \
clsname(json_t* ObjectEntry);
#define ADD_EXTERNAL_DESTRUCTORS(clsname) ~clsname();

#define RESET_INT(val)       val=0
#define RESET_UINT(val)      val=0u
#define RESET_LONGLONG(val)  val=0ll
#define RESET_ULONGLONG(val) val=0ull
#define RESET_BOOL(val)      val=false
#define RESET_DOUBLE(val)    val=0.0f
#define RESET_PTR(val)       val=nullptr

// Основные примитивы Telegram API
class TGBOT_Location;
class TGBOT_PhotoSize;
class TGBOT_Animation;
class TGBOT_MaskPosition;
class TGBOT_User;
class TGBOT_MessageEntity;
class TGBOT_Sticker;
class TGBOT_StickerSet;
class TGBOT_ChatPhoto;
class TGBOT_ChatPermissions;
class TGBOT_ChatLocation;
class TGBOT_Chat;
class TGBOT_Contact;
class TGBOT_KeyboardButton;
class TGBOT_InlineKeyboardButton;
class TGBOT_ReplyKeyboardMarkup;
class TGBOT_InlineKeyboardMarkup;
class TGBOT_ForceReply;
class TGBOT_Audio;
class TGBOT_Document;
class TGBOT_Video;
class TGBOT_VideoNote;
class TGBOT_Voice;
class TGBOT_Dice;
class TGBOT_Game;
class TGBOT_PollOption;
class TGBOT_Poll;
class TGBOT_Venue;
class TGBOT_MessageAutoDeleteTimerChanged;
class TGBOT_Invoice;
class TGBOT_ShippingAddress;
class TGBOT_OrderInfo;
class TGBOT_SuccessfulPayment;
class TGBOT_PassportFile;
class TGBOT_EncryptedCredentials;
class TGBOT_EncryptedPassportElement;
class TGBOT_PassportData;
class TGBOT_ProximityAlertTriggered;
class TGBOT_ForumTopicCreated;
class TGBOT_ForumTopicClosed;
class TGBOT_ForumTopicReopened;
class TGBOT_VideoChatScheduled;
class TGBOT_VideoChatStarted;
class TGBOT_VideoChatEnded;
class TGBOT_VideoChatParticipantsInvited;
class TGBOT_WebAppData;
class TGBOT_Message;
class TGBOT_CallbackQuery;
class TGBOT_Update;

class TGBOT_API_Class
{
	public:
		virtual ~TGBOT_API_Class() {}

		virtual void InitAll() = 0;
		virtual void FreeAll() = 0;
		// json serialize methods
		virtual void FromJSON(json_t* ObjectEntry) = 0;
		virtual SMAnsiString ToJSON() const = 0;
};

class TGBOT_Location : public TGBOT_API_Class
{
	public:
		double Longitude;
		double Latitude;
		double HorizontalAccuracy;
		int    LivePeriod;
		int    Heading;
		int    ProximityAlertRadius;

		ADD_CONSTRUCTORS(TGBOT_Location)

		virtual void InitAll() override
		{
			RESET_DOUBLE(Longitude);
			RESET_DOUBLE(Latitude);
			RESET_DOUBLE(HorizontalAccuracy);
			RESET_INT(LivePeriod);
			RESET_INT(Heading);
			RESET_INT(ProximityAlertRadius);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "longitude",              Longitude           );
			GetValueFromJSON(ObjectEntry, "latitude",               Latitude            );
			GetValueFromJSON(ObjectEntry, "horizontal_accuracy",    HorizontalAccuracy  );
			GetValueFromJSON(ObjectEntry, "live_period",            LivePeriod          );
			GetValueFromJSON(ObjectEntry, "heading",                Heading             );
			GetValueFromJSON(ObjectEntry, "proximity_alert_radius", ProximityAlertRadius);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "longitude",              Longitude           );
			PutValueToJSON(ostream, "latitude",               Latitude            );
			PutValueToJSON(ostream, "horizontal_accuracy",    HorizontalAccuracy  );
			PutValueToJSON(ostream, "live_period",            LivePeriod          );
			PutValueToJSON(ostream, "heading",                Heading             );
			PutValueToJSON(ostream, "proximity_alert_radius", ProximityAlertRadius);
			return std::move(ostream);
		}
};

class TGBOT_PhotoSize : public TGBOT_API_Class
{
	public:
		SMAnsiString FileId,
		             FileUniqueId;
		int          Width,
		             Height;
		long long    FileSize;

		ADD_CONSTRUCTORS(TGBOT_PhotoSize)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Width);
			RESET_INT(Height);
			RESET_LONGLONG(FileSize);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "width",          Width       );
			GetValueFromJSON(ObjectEntry, "height",         Height      );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_Animation : public TGBOT_API_Class
{
	public:
		SMAnsiString    FileId;
		SMAnsiString    FileUniqueId;
		int             Width;
		int             Height;
		int             Duration;
		TGBOT_PhotoSize *Thumb;
		SMAnsiString    FileName;
		SMAnsiString    MimeType;
		long long       FileSize;

		ADD_CONSTRUCTORS(TGBOT_Animation)
		ADD_DESTRUCTORS(TGBOT_Animation)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Width);
			RESET_INT(Height);
			RESET_INT(Duration);
			RESET_PTR(Thumb);
			CLEAR_STR(FileName);
			CLEAR_STR(MimeType);
			RESET_LONGLONG(FileSize);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Thumb);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId);
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "width",          Width);
			GetValueFromJSON(ObjectEntry, "height",         Height);
			GetValueFromJSON(ObjectEntry, "duration",       Duration);
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb);
			GetValueFromJSON(ObjectEntry, "file_name",      FileName);
			GetValueFromJSON(ObjectEntry, "mime_type",      MimeType);
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId);
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "width",          Width);
			PutValueToJSON(ostream, "height",         Height);
			PutValueToJSON(ostream, "duration",       Duration);
			PutValueToJSON(ostream, "thumb",          Thumb);
			PutValueToJSON(ostream, "file_name",      FileName);
			PutValueToJSON(ostream, "mime_type",      MimeType);
			PutValueToJSON(ostream, "file_size",      FileSize);
			return std::move(ostream);
		}
};

class TGBOT_MaskPosition : public TGBOT_API_Class
{
	public:
		SMAnsiString Point;
		double       XShift,
		             YShift,
		             Scale;

		ADD_CONSTRUCTORS(TGBOT_MaskPosition)

		virtual void InitAll() override
		{
			CLEAR_STR(Point);
			RESET_DOUBLE(XShift);
			RESET_DOUBLE(YShift);
			RESET_DOUBLE(Scale);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "point",   Point );
			GetValueFromJSON(ObjectEntry, "x_shift", XShift);
			GetValueFromJSON(ObjectEntry, "y_shift", YShift);
			GetValueFromJSON(ObjectEntry, "scale",   Scale );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "point",   Point );
			PutValueToJSON(ostream, "x_shift", XShift);
			PutValueToJSON(ostream, "y_shift", YShift);
			PutValueToJSON(ostream, "scale",   Scale );
			return std::move(ostream);
		}
}; 

class TGBOT_User : public TGBOT_API_Class
{
	public:
		uint64_t     Id;
		bool         Is_Bot;
		bool         IsPremium;
		SMAnsiString FirstName,
		             LastName,
		             Username,
		             LanguageCode;
		bool         AddedToAttachmentMenu;
		bool         CanJoinGroups;
		bool         CanReadAllGroupMessages;
		bool         SupportsInlineQueries;

		ADD_CONSTRUCTORS(TGBOT_User)

		virtual void InitAll() override
		{
			RESET_ULONGLONG(Id);
			RESET_BOOL(Is_Bot);
			RESET_BOOL(IsPremium);
			CLEAR_STR(FirstName);
			CLEAR_STR(LastName);
			CLEAR_STR(Username);
			CLEAR_STR(LanguageCode);
			RESET_BOOL(AddedToAttachmentMenu);
			RESET_BOOL(CanJoinGroups);
			RESET_BOOL(CanReadAllGroupMessages);
			RESET_BOOL(SupportsInlineQueries);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "id",                          Id                     );
			GetValueFromJSON(ObjectEntry, "is_bot",                      Is_Bot                 );
			GetValueFromJSON(ObjectEntry, "first_name",                  FirstName              );
			GetValueFromJSON(ObjectEntry, "last_name",                   LastName               );
			GetValueFromJSON(ObjectEntry, "username",                    Username               );
			GetValueFromJSON(ObjectEntry, "language_code",               LanguageCode           );
			GetValueFromJSON(ObjectEntry, "is_premium",                  IsPremium              );
			GetValueFromJSON(ObjectEntry, "added_to_attachment_menu",    AddedToAttachmentMenu  );
			GetValueFromJSON(ObjectEntry, "can_join_groups",             CanJoinGroups          );
			GetValueFromJSON(ObjectEntry, "can_read_all_group_messages", CanReadAllGroupMessages);
			GetValueFromJSON(ObjectEntry, "supports_inline_queries",     SupportsInlineQueries  );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",                          Id                     );
			PutValueToJSON(ostream, "is_bot",                      Is_Bot                 );
			PutValueToJSON(ostream, "first_name",                  FirstName              );
			PutValueToJSON(ostream, "last_name",                   LastName               );
			PutValueToJSON(ostream, "username",                    Username               );
			PutValueToJSON(ostream, "language_code",               LanguageCode           );
			PutValueToJSON(ostream, "is_premium",                  IsPremium              );
			PutValueToJSON(ostream, "added_to_attachment_menu",    AddedToAttachmentMenu  );
			PutValueToJSON(ostream, "can_join_groups",             CanJoinGroups          );
			PutValueToJSON(ostream, "can_read_all_group_messages", CanReadAllGroupMessages);
			PutValueToJSON(ostream, "supports_inline_queries",     SupportsInlineQueries  );
			return std::move(ostream);
		}
};

class TGBOT_MessageEntity : public TGBOT_API_Class
{
	public:
		SMAnsiString Type;
		long long    Offset;
		long long    Length;
		SMAnsiString Url;
		TGBOT_User   *User;
		SMAnsiString Language;
		SMAnsiString CustomEmojiId;

		ADD_CONSTRUCTORS(TGBOT_MessageEntity)
		ADD_DESTRUCTORS(TGBOT_MessageEntity)

		virtual void InitAll() override
		{
			CLEAR_STR(Type);
			RESET_LONGLONG(Offset);
			RESET_LONGLONG(Length);
			CLEAR_STR(Url);
			RESET_PTR(User);
			CLEAR_STR(Language);
			CLEAR_STR(CustomEmojiId);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(User);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON (ObjectEntry, "type",            Type);
			GetValueFromJSON (ObjectEntry, "offset",          Offset);
			GetValueFromJSON (ObjectEntry, "length",          Length);
			GetValueFromJSON (ObjectEntry, "url",             Url);
			GetValueFromJSON (ObjectEntry, "user",            User);
			GetValueFromJSON (ObjectEntry, "language",        Language);
			GetValueFromJSON (ObjectEntry, "custom_emoji_id", CustomEmojiId);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "type",            Type);
			PutValueToJSON(ostream, "offset",          Offset);
			PutValueToJSON(ostream, "length",          Length);
			PutValueToJSON(ostream, "url",             Url);
			PutValueToJSON(ostream, "user",            User);
			PutValueToJSON(ostream, "language",        Language);
			PutValueToJSON(ostream, "custom_emoji_id", CustomEmojiId);
			return std::move(ostream);
		}
};

class TGBOT_Sticker : public TGBOT_API_Class
{
	public:
		SMAnsiString        FileId,
		                    FileUniqueId;
		int                 Width,
		                    Height;
		bool                IsAnimated;
		TGBOT_PhotoSize     *Thumb;
		SMAnsiString        Emoji,
		                    SetName;
		TGBOT_MaskPosition* MaskPosition;
		long long           FileSize;

		ADD_CONSTRUCTORS(TGBOT_Sticker)
		ADD_DESTRUCTORS(TGBOT_Sticker)

		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(this->Thumb);
			DELETE_SINGLE_OBJECT(this->MaskPosition);
		}
		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Width);
			RESET_INT(Height);
			RESET_BOOL(IsAnimated);
			CLEAR_STR(Emoji);
			CLEAR_STR(SetName);
			RESET_LONGLONG(FileSize);
			RESET_PTR(Thumb);
			RESET_PTR(MaskPosition);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
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
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_StickerSet : public TGBOT_API_Class
{
	public:
		SMAnsiString               Name,
		                           Title;
		bool                       IsAnimated,
		                           ContainsMasks;
		TGBOT_ARRAY(TGBOT_Sticker) Stickers;

		ADD_CONSTRUCTORS(TGBOT_StickerSet)

		virtual void InitAll() override
		{
			CLEAR_STR(Name);
			CLEAR_STR(Title);
			RESET_BOOL(IsAnimated);
			RESET_BOOL(ContainsMasks);
		}
		virtual void FreeAll() override
		{
			Stickers.clear();
		}

		virtual void FromJSON(json_t * ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "name",           Name          );
			GetValueFromJSON(ObjectEntry, "title",          Title         );
			GetValueFromJSON(ObjectEntry, "is_animated",    IsAnimated    );
			GetValueFromJSON(ObjectEntry, "contains_masks", ContainsMasks );
			GetArrayFromJSON(ObjectEntry, "stickers",       Stickers      );
		}
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_ChatPhoto : public TGBOT_API_Class
{
	public:
		SMAnsiString SmallFileId;
		SMAnsiString SmallFileUniqueId;
		SMAnsiString BigFileId;
		SMAnsiString BigFileUniqueId;

		ADD_CONSTRUCTORS(TGBOT_ChatPhoto)

		virtual void InitAll() override
		{
			CLEAR_STR(SmallFileId);
			CLEAR_STR(SmallFileUniqueId);
			CLEAR_STR(BigFileId);
			CLEAR_STR(BigFileUniqueId);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "small_file_id",        SmallFileId       );
			GetValueFromJSON(ObjectEntry, "small_file_unique_id", SmallFileUniqueId );
			GetValueFromJSON(ObjectEntry, "big_file_id",          BigFileId         );
			GetValueFromJSON(ObjectEntry, "big_file_unique_id",   BigFileUniqueId   );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "small_file_id",        SmallFileId);
			PutValueToJSON(ostream, "small_file_unique_id", SmallFileUniqueId);
			PutValueToJSON(ostream, "big_file_id",          BigFileId);
			PutValueToJSON(ostream, "big_file_unique_id",   BigFileUniqueId);
			return std::move(ostream);
		}
};

class TGBOT_ChatPermissions : public TGBOT_API_Class
{
	public:
		bool CanSendMessages;
		bool CanSendMediaMessages;
		bool CanSendPolls;
		bool CanSendOtherMessages;
		bool CanAddWebPagePreviews;
		bool CanChangeInfo;
		bool CanInviteUsers;
		bool CanPinMssages;
		bool CanManageTopics;

		ADD_CONSTRUCTORS(TGBOT_ChatPermissions)

		virtual void InitAll() override
		{
			RESET_BOOL(CanSendMessages);
			RESET_BOOL(CanSendMediaMessages);
			RESET_BOOL(CanSendPolls);
			RESET_BOOL(CanSendOtherMessages);
			RESET_BOOL(CanAddWebPagePreviews);
			RESET_BOOL(CanChangeInfo);
			RESET_BOOL(CanInviteUsers);
			RESET_BOOL(CanPinMssages);
			RESET_BOOL(CanManageTopics);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "can_send_messages",         CanSendMessages);
			GetValueFromJSON(ObjectEntry, "can_send_media_messages",   CanSendMediaMessages);
			GetValueFromJSON(ObjectEntry, "can_send_polls",            CanSendPolls);
			GetValueFromJSON(ObjectEntry, "can_send_other_messages",   CanSendOtherMessages);
			GetValueFromJSON(ObjectEntry, "can_add_web_page_previews", CanAddWebPagePreviews);
			GetValueFromJSON(ObjectEntry, "can_change_info",           CanChangeInfo);
			GetValueFromJSON(ObjectEntry, "can_invite_users",          CanInviteUsers);
			GetValueFromJSON(ObjectEntry, "can_pin_messages",          CanPinMssages);
			GetValueFromJSON(ObjectEntry, "can_manage_topics",         CanManageTopics);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "can_send_messages",         CanSendMessages);
			PutValueToJSON(ostream, "can_send_media_messages",   CanSendMediaMessages);
			PutValueToJSON(ostream, "can_send_polls",            CanSendPolls);
			PutValueToJSON(ostream, "can_send_other_messages",   CanSendOtherMessages);
			PutValueToJSON(ostream, "can_add_web_page_previews", CanAddWebPagePreviews);
			PutValueToJSON(ostream, "can_change_info",           CanChangeInfo);
			PutValueToJSON(ostream, "can_invite_users",          CanInviteUsers);
			PutValueToJSON(ostream, "can_pin_messages",          CanPinMssages);
			PutValueToJSON(ostream, "can_manage_topics",         CanManageTopics);
			return std::move(ostream);
		}
};

class TGBOT_ChatLocation : public TGBOT_API_Class
{
	public:
		TGBOT_Location  *Location;
		SMAnsiString    Address;

		ADD_CONSTRUCTORS(TGBOT_ChatLocation)
		ADD_DESTRUCTORS(TGBOT_ChatLocation)

		virtual void InitAll() override
		{
			RESET_PTR(Location);
			CLEAR_STR(Address);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Location);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "location", Location);
			GetValueFromJSON(ObjectEntry, "address",  Address );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "location", Location);
			PutValueToJSON(ostream, "address",  Address );
			return std::move(ostream);
		}
};

class TGBOT_Chat : public TGBOT_API_Class
{
	public:
		uint64_t                  Id;
		SMAnsiString              Type;
		SMAnsiString              Title;
		SMAnsiString              Username;
		SMAnsiString              FirstName;
		SMAnsiString              LastName;
		bool                      IsForum;
		TGBOT_ChatPhoto           *Photo;
		TGBOT_ARRAY(SMAnsiString) ActiveUsernames;
		SMAnsiString              EmojiStatusCustomEmojiId;
		SMAnsiString              Bio;
		bool                      HasPrivateForwards;
		bool                      HasRestrictedVoiceAndVideoMessages;
		bool                      JoinToSendMessages;
		bool                      JoinByRequest;
		SMAnsiString              Description;
		SMAnsiString              InviteLink;
		TGBOT_Message             *PinnedMessage;
		TGBOT_ChatPermissions     *Permissions;
		int                       SlowModeDelay;
		int                       MessageAutoDeleteTime;
		bool                      HasProtectedContent;
		SMAnsiString              StickerSetName;
		bool                      CanSetStickerSet;
		uint64_t                  LinkedChatId;
		TGBOT_ChatLocation        *Location;

		ADD_EXTERNAL_CONSTRUCTORS(TGBOT_Chat)
		ADD_EXTERNAL_DESTRUCTORS(TGBOT_Chat)

		virtual void InitAll() override;
		virtual void FreeAll() override;

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "id",                                      Id                                );
			GetValueFromJSON(ObjectEntry, "type",                                    Type                              );
			GetValueFromJSON(ObjectEntry, "title",                                   Title                             );
			GetValueFromJSON(ObjectEntry, "first_name",                              Username                          );
			GetValueFromJSON(ObjectEntry, "last_name",                               FirstName                         );
			GetValueFromJSON(ObjectEntry, "username",                                LastName                          );
			GetValueFromJSON(ObjectEntry, "is_forum",                                IsForum                           );
			GetValueFromJSON(ObjectEntry, "photo",                                   Photo                             );
			GetArrayFromJSON(ObjectEntry, "active_usernames",                        ActiveUsernames                   );
			GetValueFromJSON(ObjectEntry, "emoji_status_custom_emoji_id",            EmojiStatusCustomEmojiId          );
			GetValueFromJSON(ObjectEntry, "bio",                                     Bio                               );
			GetValueFromJSON(ObjectEntry, "has_private_forwards",                    HasPrivateForwards                );
			GetValueFromJSON(ObjectEntry, "has_restricted_voice_and_video_messages", HasRestrictedVoiceAndVideoMessages);
			GetValueFromJSON(ObjectEntry, "join_to_send_messages",                   JoinToSendMessages                );
			GetValueFromJSON(ObjectEntry, "join_by_request",                         JoinByRequest                     );
			GetValueFromJSON(ObjectEntry, "description",                             Description                       );
			GetValueFromJSON(ObjectEntry, "invite_link",                             InviteLink                        );
			GetValueFromJSON(ObjectEntry, "pinned_message",                          PinnedMessage                     );
			GetValueFromJSON(ObjectEntry, "permissions",                             Permissions                       );
			GetValueFromJSON(ObjectEntry, "slow_mode_delay",                         SlowModeDelay                     );
			GetValueFromJSON(ObjectEntry, "message_auto_delete_time",                MessageAutoDeleteTime             );
			GetValueFromJSON(ObjectEntry, "has_protected_content",                   HasProtectedContent               );
			GetValueFromJSON(ObjectEntry, "sticker_set_name",                        StickerSetName                    );
			GetValueFromJSON(ObjectEntry, "can_set_sticker_set",                     CanSetStickerSet                  );
			GetValueFromJSON(ObjectEntry, "linked_chat_id",                          LinkedChatId                      );
			GetValueFromJSON(ObjectEntry, "location",                                Location                          );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",                                      Id                                );
			PutValueToJSON(ostream, "type",                                    Type                              );
			PutValueToJSON(ostream, "title",                                   Title                             );
			PutValueToJSON(ostream, "first_name",                              Username                          );
			PutValueToJSON(ostream, "last_name",                               FirstName                         );
			PutValueToJSON(ostream, "username",                                LastName                          );
			PutValueToJSON(ostream, "is_forum",                                IsForum                           );
			PutValueToJSON(ostream, "photo",                                   Photo                             );
			PutArrayToJSON(ostream, "active_usernames",                        ActiveUsernames                   );
			PutValueToJSON(ostream, "emoji_status_custom_emoji_id",            EmojiStatusCustomEmojiId          );
			PutValueToJSON(ostream, "bio",                                     Bio                               );
			PutValueToJSON(ostream, "has_private_forwards",                    HasPrivateForwards                );
			PutValueToJSON(ostream, "has_restricted_voice_and_video_messages", HasRestrictedVoiceAndVideoMessages);
			PutValueToJSON(ostream, "join_to_send_messages",                   JoinToSendMessages                );
			PutValueToJSON(ostream, "join_by_request",                         JoinByRequest                     );
			PutValueToJSON(ostream, "description",                             Description                       );
			PutValueToJSON(ostream, "invite_link",                             InviteLink                        );
			PutValueToJSON(ostream, "pinned_message",                          PinnedMessage                     );
			PutValueToJSON(ostream, "permissions",                             Permissions                       );
			PutValueToJSON(ostream, "slow_mode_delay",                         SlowModeDelay                     );
			PutValueToJSON(ostream, "message_auto_delete_time",                MessageAutoDeleteTime             );
			PutValueToJSON(ostream, "has_protected_content",                   HasProtectedContent               );
			PutValueToJSON(ostream, "sticker_set_name",                        StickerSetName                    );
			PutValueToJSON(ostream, "can_set_sticker_set",                     CanSetStickerSet                  );
			PutValueToJSON(ostream, "linked_chat_id",                          LinkedChatId                      );
			PutValueToJSON(ostream, "location",                                Location                          );
			return std::move(ostream);
		}
};

class TGBOT_Contact : public TGBOT_API_Class
{
	public:
		SMAnsiString PhoneNumber,
		             FirstName,
		             LastName;
		uint64_t     UserId;
		SMAnsiString Vcard;

		ADD_CONSTRUCTORS(TGBOT_Contact)

		virtual void InitAll() override
		{
			CLEAR_STR(PhoneNumber);
			CLEAR_STR(FirstName);
			CLEAR_STR(LastName);
			RESET_ULONGLONG(UserId);
			CLEAR_STR(Vcard);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "phone_number",  PhoneNumber);
			GetValueFromJSON(ObjectEntry, "first_name",    FirstName  );
			GetValueFromJSON(ObjectEntry, "last_name",     LastName   );
			GetValueFromJSON(ObjectEntry, "user_id",       UserId     );
			GetValueFromJSON(ObjectEntry, "vcard",         Vcard      );
		}
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_KeyboardButton : public TGBOT_API_Class
{
	public:
		SMAnsiString Text;
		bool         RequestContact,
		             RequestLocation;

		ADD_CONSTRUCTORS(TGBOT_KeyboardButton)
		TGBOT_KeyboardButton(const TGBOT_KeyboardButton &val);
		TGBOT_KeyboardButton(const TGBOT_KeyboardButton *val);
		TGBOT_KeyboardButton(SMAnsiString Text, bool RequestContact, bool RequestLocation);

		virtual void InitAll() override
		{
			CLEAR_STR(Text);
			RESET_BOOL(RequestContact);
			RESET_BOOL(RequestLocation);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "text",             Text           );
			GetValueFromJSON(ObjectEntry, "request_contact",  RequestContact );
			GetValueFromJSON(ObjectEntry, "request_location", RequestLocation);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "text",             Text           );
			PutValueToJSON(ostream, "request_contact",  RequestContact );
			PutValueToJSON(ostream, "request_location", RequestLocation);
			return std::move(ostream);
		}
};

class TGBOT_InlineKeyboardButton : public TGBOT_API_Class
{
	public:
		SMAnsiString Text;
		SMAnsiString Url;
		SMAnsiString CallbackData;
		SMAnsiString SwitchInlineQuery;
		SMAnsiString SwitchInlineQueryCurrentChat;
		bool         Pay;

		ADD_CONSTRUCTORS(TGBOT_InlineKeyboardButton)
		TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton *b);
		TGBOT_InlineKeyboardButton(const TGBOT_InlineKeyboardButton &b);
		TGBOT_InlineKeyboardButton(SMAnsiString Text) { CustomInit(Text, "", "", false); }
		TGBOT_InlineKeyboardButton(SMAnsiString Text, SMAnsiString Url) { CustomInit(Text, Url, "", false); }
		TGBOT_InlineKeyboardButton(SMAnsiString Text, SMAnsiString CallbackData, bool Pay) { CustomInit(Text, "", CallbackData, Pay); }

		virtual void InitAll() override
		{
			CLEAR_STR(Text);
			CLEAR_STR(Url);
			CLEAR_STR(CallbackData);
			CLEAR_STR(SwitchInlineQuery);
			CLEAR_STR(SwitchInlineQueryCurrentChat);
			RESET_BOOL(Pay);
		}
		virtual void FreeAll() override {}

		void CustomInit(SMAnsiString Text, SMAnsiString Url, SMAnsiString CallbackData, bool Pay);

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "text",                             Text                        );
			GetValueFromJSON(ObjectEntry, "url",                              Url                         );
			GetValueFromJSON(ObjectEntry, "callback_data",                    CallbackData                );
			GetValueFromJSON(ObjectEntry, "switch_inline_query",              SwitchInlineQuery           );
			GetValueFromJSON(ObjectEntry, "switch_inline_query_current_chat", SwitchInlineQueryCurrentChat);
			GetValueFromJSON(ObjectEntry, "pay",                              Pay                         );
		}
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_Keyboard : public TGBOT_API_Class
{
	public:
		virtual ~TGBOT_Keyboard() {}
};

class TGBOT_ReplyKeyboardMarkup: public TGBOT_Keyboard
{
	private:
		TGBOT_ARRAY_OF_ARRAY<TGBOT_KeyboardButton> Keyboard;
		bool                                       ResizeKeyboard,
		                                           OneTimeKeyboard,
		                                           Selective;

	public:
		ADD_CONSTRUCTORS(TGBOT_ReplyKeyboardMarkup)
		TGBOT_ReplyKeyboardMarkup(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective) { CustomInit(ResizeKeyboard, OneTimeKeyboard, Selective, false); }

		virtual void InitAll() override
		{
			RESET_BOOL(ResizeKeyboard);
			RESET_BOOL(OneTimeKeyboard);
			RESET_BOOL(Selective);
		}
		virtual void FreeAll() override { Keyboard.clear(); }

		void CustomInit(bool ResizeKeyboard, bool OneTimeKeyboard, bool Selective, bool freemem = true);
		void CreateButton(SMAnsiString Text);
		void CreateRow() { Keyboard.CreateRow(); }

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON       (ObjectEntry, "resize_keyboard",   ResizeKeyboard );
			GetValueFromJSON       (ObjectEntry, "one_time_keyboard", OneTimeKeyboard);
			GetValueFromJSON       (ObjectEntry, "selective",         Selective      );
			GetArrayOfArrayFromJSON(ObjectEntry, "keyboard",          Keyboard       );
		}
		virtual SMAnsiString ToJSON() const override
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

		ADD_CONSTRUCTORS(TGBOT_InlineKeyboardMarkup)
		TGBOT_InlineKeyboardMarkup(const TGBOT_InlineKeyboardMarkup& kb) { InlineKeyboard = kb.InlineKeyboard; }

		virtual void InitAll() override {}
		virtual void FreeAll() override { InlineKeyboard.clear(); }

		void CreateButton(SMAnsiString Text) { InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text)); }
		void CreateButton(SMAnsiString Text, SMAnsiString Url) { InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text, Url)); }
		void CreateButton(SMAnsiString Text, SMAnsiString CallbackData, bool Pay) { InlineKeyboard.push_back(new TGBOT_InlineKeyboardButton(Text, CallbackData, Pay)); }
		void CreateRow() { InlineKeyboard.CreateRow(); }

		virtual void FromJSON(json_t * ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetArrayOfArrayFromJSON(ObjectEntry, "inline_keyboard", InlineKeyboard);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutArrayOfArrayToJSON(ostream, "inline_keyboard", InlineKeyboard);
			return std::move(ostream);
		}
};

class TGBOT_ForceReply : public TGBOT_API_Class
{
	public:
		bool ForceReply,
		     Selective;

		ADD_CONSTRUCTORS(TGBOT_ForceReply)

		virtual void InitAll() override
		{
			RESET_BOOL(ForceReply);
			RESET_BOOL(Selective);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "force_reply", ForceReply);
			GetValueFromJSON(ObjectEntry, "selective",   Selective );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "force_reply", ForceReply);
			PutValueToJSON(ostream, "selective",   Selective );
			return std::move(ostream);
		}
};

class TGBOT_Audio : public TGBOT_API_Class
{
	public:
		SMAnsiString     FileId;
		SMAnsiString     FileUniqueId;
		int              Duration;
		SMAnsiString     Performer;
		SMAnsiString     Title;
		SMAnsiString     FileName;
		SMAnsiString     MimeType;
		size_t           FileSize;
		TGBOT_PhotoSize* Thumb;

		ADD_CONSTRUCTORS(TGBOT_Audio)
		ADD_DESTRUCTORS(TGBOT_Audio)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Duration);
			CLEAR_STR(Performer);
			CLEAR_STR(Title);
			CLEAR_STR(FileName);
			CLEAR_STR(MimeType);
			RESET_ULONGLONG(FileSize);
			RESET_PTR(Thumb);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Thumb);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "duration",       Duration    );
			GetValueFromJSON(ObjectEntry, "performer",      Performer   );
			GetValueFromJSON(ObjectEntry, "title",          Title       );
			GetValueFromJSON(ObjectEntry, "file_name",      FileName    );
			GetValueFromJSON(ObjectEntry, "mime_type",      MimeType    );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb       );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "duration",       Duration    );
			PutValueToJSON(ostream, "performer",      Performer   );
			PutValueToJSON(ostream, "title",          Title       );
			PutValueToJSON(ostream, "file_name",      FileName    );
			PutValueToJSON(ostream, "mime_type",      MimeType    );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			PutValueToJSON(ostream, "thumb",          Thumb       );
			return std::move(ostream);
		}
};

class TGBOT_Document : public TGBOT_API_Class
{
	public:
		SMAnsiString     FileId;
		SMAnsiString     FileUniqueId;
		TGBOT_PhotoSize* Thumb;
		SMAnsiString     FileName;
		SMAnsiString     MimeType;
		size_t           FileSize;

		ADD_CONSTRUCTORS(TGBOT_Document)
		ADD_DESTRUCTORS(TGBOT_Document)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_PTR(Thumb);
			CLEAR_STR(FileName);
			CLEAR_STR(MimeType);
			RESET_ULONGLONG(FileSize);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Thumb);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb       );
			GetValueFromJSON(ObjectEntry, "file_name",      FileName    );
			GetValueFromJSON(ObjectEntry, "mime_type",      MimeType    );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "thumb",          Thumb       );
			PutValueToJSON(ostream, "file_name",      FileName    );
			PutValueToJSON(ostream, "mime_type",      MimeType    );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			return std::move(ostream);
		}
};

class TGBOT_Video : public TGBOT_API_Class
{
	public:
		SMAnsiString     FileId;
		SMAnsiString     FileUniqueId;
		int              Width;
		int              Height;
		int              Duration;
		TGBOT_PhotoSize* Thumb;
		SMAnsiString     FileName;
		SMAnsiString     MimeType;
		size_t           FileSize;

		ADD_CONSTRUCTORS(TGBOT_Video)
		ADD_DESTRUCTORS(TGBOT_Video)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Width);
			RESET_INT(Height);
			RESET_INT(Duration);
			RESET_PTR(Thumb);
			CLEAR_STR(FileName);
			CLEAR_STR(MimeType);
			RESET_ULONGLONG(FileSize);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Thumb);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "width",          Width       );
			GetValueFromJSON(ObjectEntry, "height",         Height      );
			GetValueFromJSON(ObjectEntry, "duration",       Duration    );
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb       );
			GetValueFromJSON(ObjectEntry, "file_name",      FileName    );
			GetValueFromJSON(ObjectEntry, "mime_type",      MimeType    );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "width",          Width       );
			PutValueToJSON(ostream, "height",         Height      );
			PutValueToJSON(ostream, "duration",       Duration    );
			PutValueToJSON(ostream, "thumb",          Thumb       );
			PutValueToJSON(ostream, "file_name",      FileName    );
			PutValueToJSON(ostream, "mime_type",      MimeType    );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			return std::move(ostream);
		}
};

class TGBOT_VideoNote : public TGBOT_API_Class
{
	public:
		SMAnsiString     FileId;
		SMAnsiString     FileUniqueId;
		int              Length;
		int              Duration;
		TGBOT_PhotoSize* Thumb;
		size_t           FileSize;

		ADD_CONSTRUCTORS(TGBOT_VideoNote)
		ADD_DESTRUCTORS(TGBOT_VideoNote)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Length);
			RESET_INT(Duration);
			RESET_PTR(Thumb);
			RESET_ULONGLONG(FileSize);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Thumb);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "length",         Length      );
			GetValueFromJSON(ObjectEntry, "duration",       Duration    );
			GetValueFromJSON(ObjectEntry, "thumb",          Thumb       );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "length",         Length      );
			PutValueToJSON(ostream, "duration",       Duration    );
			PutValueToJSON(ostream, "thumb",          Thumb       );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			return std::move(ostream);
		}
};

class TGBOT_Voice : public TGBOT_API_Class
{
	public:
		SMAnsiString     FileId;
		SMAnsiString     FileUniqueId;
		int              Duration;
		SMAnsiString     MimeType;
		size_t           FileSize;

		ADD_CONSTRUCTORS(TGBOT_Voice)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_INT(Duration);
			CLEAR_STR(MimeType);
			RESET_ULONGLONG(FileSize);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "duration",       Duration    );
			GetValueFromJSON(ObjectEntry, "mime_type",      MimeType    );
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "duration",       Duration    );
			PutValueToJSON(ostream, "mime_type",      MimeType    );
			PutValueToJSON(ostream, "file_size",      FileSize    );
			return std::move(ostream);
		}
};

class TGBOT_Dice : public TGBOT_API_Class
{
	public:
		SMAnsiString Emoji;
		int          Value;

		ADD_CONSTRUCTORS(TGBOT_Dice)

		virtual void InitAll() override
		{
			CLEAR_STR(Emoji);
			RESET_INT(Value);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "emoji", Emoji);
			GetValueFromJSON(ObjectEntry, "value", Value);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "emoji", Emoji);
			PutValueToJSON(ostream, "value", Value);
			return std::move(ostream);
		}
};

class TGBOT_Game : public TGBOT_API_Class
{
	public:
		SMAnsiString                     Title;
		SMAnsiString                     Description;
		TGBOT_ARRAY(TGBOT_PhotoSize)     Photo;
		SMAnsiString                     Text;
		TGBOT_ARRAY(TGBOT_MessageEntity) TextEntities;
		TGBOT_Animation*                 Animation;

		ADD_CONSTRUCTORS(TGBOT_Game)

		virtual void InitAll() override
		{
			CLEAR_STR(Title);
			CLEAR_STR(Description);
			CLEAR_STR(Text);
			RESET_PTR(Animation);
		}
		virtual void FreeAll() override
		{
			Photo.clear();
			TextEntities.clear();
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "title",         Title         );
			GetValueFromJSON(ObjectEntry, "description",   Description   );
			GetArrayFromJSON(ObjectEntry, "photo",         Photo         );
			GetValueFromJSON(ObjectEntry, "text",          Text          );
			GetArrayFromJSON(ObjectEntry, "text_entities", TextEntities  );
			GetValueFromJSON(ObjectEntry, "animation",     Animation     );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "title",         Title         );
			PutValueToJSON(ostream, "description",   Description   );
			PutArrayToJSON(ostream, "photo",         Photo         );
			PutValueToJSON(ostream, "text",          Text          );
			PutArrayToJSON(ostream, "text_entities", TextEntities  );
			PutValueToJSON(ostream, "animation",     Animation     );
			return std::move(ostream);
		}
};

class TGBOT_PollOption : public TGBOT_API_Class
{
	public:
		SMAnsiString Id;
		uint64_t     VoterCount;

		ADD_CONSTRUCTORS(TGBOT_PollOption)

		virtual void InitAll() override
		{
			CLEAR_STR(Id);
			RESET_ULONGLONG(VoterCount);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "id",          Id        );
			GetValueFromJSON(ObjectEntry, "voter_count", VoterCount);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",          Id        );
			PutValueToJSON(ostream, "voter_count", VoterCount);
			return std::move(ostream);
		}
};

class TGBOT_Poll : public TGBOT_API_Class
{
	public:
		SMAnsiString                     Id;
		SMAnsiString                     Question;
		TGBOT_ARRAY(TGBOT_PollOption)    Options;
		uint64_t                         TotalVoterCount;
		bool                             IsClosed;
		bool                             IsAnonymous;
		SMAnsiString                     Type;
		bool                             AllowsMultipleAnswers;
		uint64_t                         CorrectOptionId;
		SMAnsiString                     Explanation;
		TGBOT_ARRAY(TGBOT_MessageEntity) ExplanationEntities;
		long long                        OpenPeriod;
		time_t                           CloseDate;

		ADD_CONSTRUCTORS(TGBOT_Poll)

		virtual void InitAll() override
		{
			CLEAR_STR(Id);
			CLEAR_STR(Question);
			RESET_ULONGLONG(TotalVoterCount);
			RESET_BOOL(IsClosed);
			RESET_BOOL(IsAnonymous);
			CLEAR_STR(Type);
			RESET_BOOL(AllowsMultipleAnswers);
			RESET_ULONGLONG(CorrectOptionId);
			CLEAR_STR(Explanation);
			RESET_LONGLONG(OpenPeriod);
			RESET_INT(CloseDate);
		}
		virtual void FreeAll() override
		{
			Options.clear();
			ExplanationEntities.clear();
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "id",                      Id                   );
			GetValueFromJSON(ObjectEntry, "question",                Question             );
			GetArrayFromJSON(ObjectEntry, "option",                  Options              );
			GetValueFromJSON(ObjectEntry, "total_voter_count",       TotalVoterCount      );
			GetValueFromJSON(ObjectEntry, "is_closed",               IsClosed             );
			GetValueFromJSON(ObjectEntry, "is_anonymous",            IsAnonymous          );
			GetValueFromJSON(ObjectEntry, "type",                    Type                 );
			GetValueFromJSON(ObjectEntry, "allows_multiple_answers", AllowsMultipleAnswers);
			GetValueFromJSON(ObjectEntry, "correct_option_id",       CorrectOptionId      );
			GetValueFromJSON(ObjectEntry, "explanation",             Explanation          );
			GetArrayFromJSON(ObjectEntry, "explanation_entities",    ExplanationEntities  );
			GetValueFromJSON(ObjectEntry, "open_period",             OpenPeriod           );
			GetValueFromJSON(ObjectEntry, "close_date",              CloseDate            );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "id",                      Id                   );
			PutValueToJSON(ostream, "question",                Question             );
			PutArrayToJSON(ostream, "option",                  Options              );
			PutValueToJSON(ostream, "total_voter_count",       TotalVoterCount      );
			PutValueToJSON(ostream, "is_closed",               IsClosed             );
			PutValueToJSON(ostream, "is_anonymous",            IsAnonymous          );
			PutValueToJSON(ostream, "type",                    Type                 );
			PutValueToJSON(ostream, "allows_multiple_answers", AllowsMultipleAnswers);
			PutValueToJSON(ostream, "correct_option_id",       CorrectOptionId      );
			PutValueToJSON(ostream, "explanation",             Explanation          );
			PutArrayToJSON(ostream, "explanation_entities",    ExplanationEntities  );
			PutValueToJSON(ostream, "open_period",             OpenPeriod           );
			PutValueToJSON(ostream, "close_date",              CloseDate            );
			return std::move(ostream);
		}
};

class TGBOT_Venue : public TGBOT_API_Class
{
	public:
		TGBOT_Location* Location;
		SMAnsiString    Title;
		SMAnsiString    Address;
		SMAnsiString    FoursquareId;
		SMAnsiString    FoursquareType;
		SMAnsiString    GooglePlaceId;
		SMAnsiString    GooglePlaceType;

		ADD_CONSTRUCTORS(TGBOT_Venue)

		virtual void InitAll() override
		{
			RESET_PTR(Location);
			CLEAR_STR(Title);
			CLEAR_STR(Address);
			CLEAR_STR(FoursquareId);
			CLEAR_STR(FoursquareType);
			CLEAR_STR(GooglePlaceId);
			CLEAR_STR(GooglePlaceType);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Location);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "location",          Location       );
			GetValueFromJSON(ObjectEntry, "title",             Title          );
			GetValueFromJSON(ObjectEntry, "address",           Address        );
			GetValueFromJSON(ObjectEntry, "foursquare_id",     FoursquareId   );
			GetValueFromJSON(ObjectEntry, "foursquare_type",   FoursquareType );
			GetValueFromJSON(ObjectEntry, "google_place_id",   GooglePlaceId  );
			GetValueFromJSON(ObjectEntry, "google_place_type", GooglePlaceType);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "location",          Location       );
			PutValueToJSON(ostream, "title",             Title          );
			PutValueToJSON(ostream, "address",           Address        );
			PutValueToJSON(ostream, "foursquare_id",     FoursquareId   );
			PutValueToJSON(ostream, "foursquare_type",   FoursquareType );
			PutValueToJSON(ostream, "google_place_id",   GooglePlaceId  );
			PutValueToJSON(ostream, "google_place_type", GooglePlaceType);
			return std::move(ostream);
		}
};

class TGBOT_MessageAutoDeleteTimerChanged : public TGBOT_API_Class
{
	public:
		long long MessageAutoDeleteTime;

		ADD_CONSTRUCTORS(TGBOT_MessageAutoDeleteTimerChanged)

		virtual void InitAll() override
		{
			RESET_LONGLONG(MessageAutoDeleteTime);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "message_auto_delete_time", MessageAutoDeleteTime);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "message_auto_delete_time", MessageAutoDeleteTime);
			return std::move(ostream);
		}
};

class TGBOT_Invoice : public TGBOT_API_Class
{
	public:
		SMAnsiString Title;
		SMAnsiString Description;
		SMAnsiString StartParameter;
		SMAnsiString Currency;
		long long    TotalAmount;

		ADD_CONSTRUCTORS(TGBOT_Invoice)

		virtual void InitAll() override
		{
			CLEAR_STR(Title);
			CLEAR_STR(Description);
			CLEAR_STR(StartParameter);
			CLEAR_STR(Currency);
			RESET_LONGLONG(TotalAmount);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "title",           Title         );
			GetValueFromJSON(ObjectEntry, "description",     Description   );
			GetValueFromJSON(ObjectEntry, "start_parameter", StartParameter);
			GetValueFromJSON(ObjectEntry, "currency",        Currency      );
			GetValueFromJSON(ObjectEntry, "total_amount",    TotalAmount   );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "title",           Title         );
			PutValueToJSON(ostream, "description",     Description   );
			PutValueToJSON(ostream, "start_parameter", StartParameter);
			PutValueToJSON(ostream, "currency",        Currency      );
			PutValueToJSON(ostream, "total_amount",    TotalAmount   );
			return std::move(ostream);
		}
};

class TGBOT_ShippingAddress : public TGBOT_API_Class
{
	public:
		SMAnsiString CountryCode;
		SMAnsiString State;
		SMAnsiString City;
		SMAnsiString StreetLine1;
		SMAnsiString StreetLine2;
		SMAnsiString PostCode;

		ADD_CONSTRUCTORS(TGBOT_ShippingAddress)

		virtual void InitAll() override
		{
			CLEAR_STR(CountryCode);
			CLEAR_STR(State);
			CLEAR_STR(City);
			CLEAR_STR(StreetLine1);
			CLEAR_STR(StreetLine2);
			CLEAR_STR(PostCode);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "country_code", CountryCode);
			GetValueFromJSON(ObjectEntry, "state",        State      );
			GetValueFromJSON(ObjectEntry, "city",         City       );
			GetValueFromJSON(ObjectEntry, "street_line1", StreetLine1);
			GetValueFromJSON(ObjectEntry, "street_line2", StreetLine2);
			GetValueFromJSON(ObjectEntry, "post_code",    PostCode   );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "country_code", CountryCode);
			PutValueToJSON(ostream, "state",        State      );
			PutValueToJSON(ostream, "city",         City       );
			PutValueToJSON(ostream, "street_line1", StreetLine1);
			PutValueToJSON(ostream, "street_line2", StreetLine2);
			PutValueToJSON(ostream, "post_code",    PostCode   );
			return std::move(ostream);
		}
};

class TGBOT_OrderInfo : public TGBOT_API_Class
{
	public:
		SMAnsiString           Name;
		SMAnsiString           PhoneNumber;
		SMAnsiString           Email;
		TGBOT_ShippingAddress* ShippingAddress;

		ADD_CONSTRUCTORS(TGBOT_OrderInfo)
		ADD_DESTRUCTORS(TGBOT_OrderInfo)

		virtual void InitAll() override
		{
			CLEAR_STR(Name);
			CLEAR_STR(PhoneNumber);
			CLEAR_STR(Email);
			RESET_PTR(ShippingAddress);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(ShippingAddress);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "name",             Name           );
			GetValueFromJSON(ObjectEntry, "phone_number",     PhoneNumber    );
			GetValueFromJSON(ObjectEntry, "email",            Email          );
			GetValueFromJSON(ObjectEntry, "shipping_address", ShippingAddress);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "name",             Name           );
			PutValueToJSON(ostream, "phone_number",     PhoneNumber    );
			PutValueToJSON(ostream, "email",            Email          );
			PutValueToJSON(ostream, "shipping_address", ShippingAddress);
			return std::move(ostream);
		}
};

class TGBOT_SuccessfulPayment : public TGBOT_API_Class
{
	public:
		SMAnsiString           Currency;
		long long              TotalAmount;
		SMAnsiString           InvoicePayload;
		SMAnsiString           ShippingOptionId;
		TGBOT_OrderInfo*       OrderInfo;
		SMAnsiString           TelegramPaymentChargeId;
		SMAnsiString           ProviderPaymentChargeId;

		ADD_CONSTRUCTORS(TGBOT_SuccessfulPayment)
		ADD_DESTRUCTORS(TGBOT_SuccessfulPayment)

		virtual void InitAll() override
		{
			CLEAR_STR(Currency);
			RESET_LONGLONG(TotalAmount);
			CLEAR_STR(InvoicePayload);
			CLEAR_STR(ShippingOptionId);
			RESET_PTR(OrderInfo);
			CLEAR_STR(TelegramPaymentChargeId);
			CLEAR_STR(ProviderPaymentChargeId);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(OrderInfo);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "currency",                   Currency               );
			GetValueFromJSON(ObjectEntry, "total_amount",               TotalAmount            );
			GetValueFromJSON(ObjectEntry, "invoice_payload",            InvoicePayload         );
			GetValueFromJSON(ObjectEntry, "shipping_option_id",         ShippingOptionId       );
			GetValueFromJSON(ObjectEntry, "order_info",                 OrderInfo              );
			GetValueFromJSON(ObjectEntry, "telegram_payment_charge_id", TelegramPaymentChargeId);
			GetValueFromJSON(ObjectEntry, "provider_payment_charge_id", ProviderPaymentChargeId);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "currency",                   Currency               );
			PutValueToJSON(ostream, "total_amount",               TotalAmount            );
			PutValueToJSON(ostream, "invoice_payload",            InvoicePayload         );
			PutValueToJSON(ostream, "shipping_option_id",         ShippingOptionId       );
			PutValueToJSON(ostream, "order_info",                 OrderInfo              );
			PutValueToJSON(ostream, "telegram_payment_charge_id", TelegramPaymentChargeId);
			PutValueToJSON(ostream, "provider_payment_charge_id", ProviderPaymentChargeId);
			return std::move(ostream);
		}
};

class TGBOT_EncryptedCredentials : public TGBOT_API_Class
{
	public:
		SMAnsiString Data;
		SMAnsiString Hash;
		SMAnsiString Secret;

		ADD_CONSTRUCTORS(TGBOT_EncryptedCredentials)

		virtual void InitAll() override
		{
			CLEAR_STR(Data);
			CLEAR_STR(Hash);
			CLEAR_STR(Secret);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "data",   Data  );
			GetValueFromJSON(ObjectEntry, "hash",   Hash  );
			GetValueFromJSON(ObjectEntry, "secret", Secret);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "data",   Data  );
			PutValueToJSON(ostream, "hash",   Hash  );
			PutValueToJSON(ostream, "secret", Secret);
			return std::move(ostream);
		}
};

class TGBOT_PassportFile : public TGBOT_API_Class
{
	public:
		SMAnsiString FileId;
		SMAnsiString FileUniqueId;
		uint64_t     FileSize;
		time_t       FileDate;

		ADD_CONSTRUCTORS(TGBOT_PassportFile)

		virtual void InitAll() override
		{
			CLEAR_STR(FileId);
			CLEAR_STR(FileUniqueId);
			RESET_ULONGLONG(FileSize);
			RESET_INT(FileDate);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "file_id",        FileId      );
			GetValueFromJSON(ObjectEntry, "file_unique_id", FileUniqueId);
			GetValueFromJSON(ObjectEntry, "file_size",      FileSize    );
			GetValueFromJSON(ObjectEntry, "file_date",      FileDate    );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "file_id",        FileId      );
			PutValueToJSON(ostream, "file_unique_id", FileUniqueId);
			PutValueToJSON(ostream, "file_size",      FileSize    );
			PutValueToJSON(ostream, "file_date",      FileDate    );
			return std::move(ostream);
		}
};

class TGBOT_EncryptedPassportElement : public TGBOT_API_Class
{
	public:
		SMAnsiString                    Type;
		SMAnsiString                    Data;
		SMAnsiString                    PhoneNumber;
		SMAnsiString                    Email;
		TGBOT_ARRAY(TGBOT_PassportFile) Files;
		TGBOT_PassportFile*             FrontSide;
		TGBOT_PassportFile*             ReverseSide;
		TGBOT_PassportFile*             Selfie;
		TGBOT_ARRAY(TGBOT_PassportFile) Translation;
		SMAnsiString                    Hash;

		ADD_CONSTRUCTORS(TGBOT_EncryptedPassportElement)
		ADD_DESTRUCTORS(TGBOT_EncryptedPassportElement)

		virtual void InitAll() override
		{
			CLEAR_STR(Type);
			CLEAR_STR(Data);
			CLEAR_STR(PhoneNumber);
			CLEAR_STR(Email);
			RESET_PTR(FrontSide);
			RESET_PTR(ReverseSide);
			RESET_PTR(Selfie);
			CLEAR_STR(Hash);
		}
		virtual void FreeAll() override
		{
			Files.clear();
			Translation.clear();
			DELETE_SINGLE_OBJECT(FrontSide);
			DELETE_SINGLE_OBJECT(ReverseSide);
			DELETE_SINGLE_OBJECT(Selfie);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "type",         Type);
			GetValueFromJSON(ObjectEntry, "data",         Data);
			GetValueFromJSON(ObjectEntry, "phone_number", PhoneNumber);
			GetValueFromJSON(ObjectEntry, "email",        Email);
			GetArrayFromJSON(ObjectEntry, "files",        Files);
			GetValueFromJSON(ObjectEntry, "front_side",   FrontSide);
			GetValueFromJSON(ObjectEntry, "reverse_side", ReverseSide);
			GetValueFromJSON(ObjectEntry, "selfie",       Selfie);
			GetArrayFromJSON(ObjectEntry, "translation",  Translation);
			GetValueFromJSON(ObjectEntry, "hash",         Hash);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "type",         Type);
			PutValueToJSON(ostream, "data",         Data);
			PutValueToJSON(ostream, "phone_number", PhoneNumber);
			PutValueToJSON(ostream, "email",        Email);
			PutArrayToJSON(ostream, "files",        Files);
			PutValueToJSON(ostream, "front_side",   FrontSide);
			PutValueToJSON(ostream, "reverse_side", ReverseSide);
			PutValueToJSON(ostream, "selfie",       Selfie);
			PutArrayToJSON(ostream, "translation",  Translation);
			PutValueToJSON(ostream, "hash",         Hash);
			return std::move(ostream);
		}
};

class TGBOT_PassportData : public TGBOT_API_Class
{
	public:
		TGBOT_ARRAY(TGBOT_EncryptedPassportElement) Data;
		TGBOT_EncryptedCredentials*                 Credentials;

		ADD_CONSTRUCTORS(TGBOT_PassportData)
		ADD_DESTRUCTORS(TGBOT_PassportData)

		virtual void InitAll() override
		{
			RESET_PTR(Credentials);
		}
		virtual void FreeAll() override
		{
			Data.clear();
			DELETE_SINGLE_OBJECT(Credentials);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetArrayFromJSON(ObjectEntry, "data",        Data       );
			GetValueFromJSON(ObjectEntry, "credentials", Credentials);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutArrayToJSON(ostream, "data",        Data       );
			PutValueToJSON(ostream, "credentials", Credentials);
			return std::move(ostream);
		}
};

class TGBOT_ProximityAlertTriggered : public TGBOT_API_Class
{
	public:
		TGBOT_User* Traveler;
		TGBOT_User* Watcher;
		int         Distance;

		ADD_CONSTRUCTORS(TGBOT_ProximityAlertTriggered)
		ADD_DESTRUCTORS(TGBOT_ProximityAlertTriggered)

		virtual void InitAll() override
		{
			RESET_PTR(Traveler);
			RESET_PTR(Watcher);
			RESET_INT(Distance);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(Traveler);
			DELETE_SINGLE_OBJECT(Watcher);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "traveler", Traveler);
			GetValueFromJSON(ObjectEntry, "watcher",  Watcher );
			GetValueFromJSON(ObjectEntry, "distance", Distance);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "traveler", Traveler);
			PutValueToJSON(ostream, "watcher",  Watcher );
			PutValueToJSON(ostream, "distance", Distance);
			return std::move(ostream);
		}
};

class TGBOT_ForumTopicCreated : public TGBOT_API_Class
{
	public:
		SMAnsiString Name;
		int          IconColor;
		SMAnsiString IconCustomEmojiId;

		ADD_CONSTRUCTORS(TGBOT_ForumTopicCreated)

		virtual void InitAll() override
		{
			CLEAR_STR(Name);
			RESET_INT(IconColor);
			CLEAR_STR(IconCustomEmojiId);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "name",                 Name             );
			GetValueFromJSON(ObjectEntry, "icon_color",           IconColor        );
			GetValueFromJSON(ObjectEntry, "icon_custom_emoji_id", IconCustomEmojiId);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "name",                 Name             );
			PutValueToJSON(ostream, "icon_color",           IconColor        );
			PutValueToJSON(ostream, "icon_custom_emoji_id", IconCustomEmojiId);
			return std::move(ostream);
		}
};

// TODO: пока не содержит инфы, зайди в документацию и проверь актуальность
class TGBOT_ForumTopicClosed : public TGBOT_API_Class
{
	public:
		int dummy;

		ADD_CONSTRUCTORS(TGBOT_ForumTopicClosed)

		virtual void InitAll() override {}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override {}
		virtual SMAnsiString ToJSON() const override { return SMAnsiString("{}"); }
};

// TODO: пока не содержит инфы, зайди в документацию и проверь актуальность
class TGBOT_ForumTopicReopened : public TGBOT_API_Class
{
	public:
		int dummy;

		ADD_CONSTRUCTORS(TGBOT_ForumTopicReopened)

		virtual void InitAll() override {}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override {}
		virtual SMAnsiString ToJSON() const override { return SMAnsiString("{}"); }
};

class TGBOT_VideoChatScheduled : public TGBOT_API_Class
{
	public:
		time_t StartDate;

		ADD_CONSTRUCTORS(TGBOT_VideoChatScheduled)

		virtual void InitAll() override
		{
			RESET_INT(StartDate);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "start_date", StartDate);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "start_date", StartDate);
			return std::move(ostream);
		}
};

// TODO: пока не содержит инфы, зайди в документацию и проверь актуальность
class TGBOT_VideoChatStarted : public TGBOT_API_Class
{
	public:
		int dummy;

		ADD_CONSTRUCTORS(TGBOT_VideoChatStarted)

		virtual void InitAll() override {}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override {}
		virtual SMAnsiString ToJSON() const override { return SMAnsiString("{}"); }
};

class TGBOT_VideoChatEnded : public TGBOT_API_Class
{
	public:
		int Duration;

		ADD_CONSTRUCTORS(TGBOT_VideoChatEnded)

		virtual void InitAll() override
		{
			RESET_INT(Duration);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "duration", Duration);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "duration", Duration);
			return std::move(ostream);
		}
};

class TGBOT_VideoChatParticipantsInvited : public TGBOT_API_Class
{
	public:
		TGBOT_ARRAY(TGBOT_User) Users;

		ADD_CONSTRUCTORS(TGBOT_VideoChatParticipantsInvited)

		virtual void InitAll() override {}
		virtual void FreeAll() override 
		{
			Users.clear();
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetArrayFromJSON(ObjectEntry, "users", Users);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutArrayToJSON(ostream, "users", Users);
			return std::move(ostream);
		}
};

class TGBOT_WebAppData : public TGBOT_API_Class
{
	public:
		SMAnsiString Data;
		SMAnsiString ButtonText;

		ADD_CONSTRUCTORS(TGBOT_WebAppData)

		virtual void InitAll() override 
		{
			CLEAR_STR(Data);
			CLEAR_STR(ButtonText);
		}
		virtual void FreeAll() override {}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "data",        Data      );
			GetValueFromJSON(ObjectEntry, "button_text", ButtonText);
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "data",        Data      );
			PutValueToJSON(ostream, "button_text", ButtonText);
			return std::move(ostream);
		}
};

class TGBOT_Message : public TGBOT_API_Class
{
	public:
		uint64_t                             MessageId;
		uint64_t                             MessageThreadId;
		TGBOT_User*                          From;
		TGBOT_Chat*                          SenderChat;
		time_t                               Date;
		TGBOT_Chat*                          Chat;
		TGBOT_User*                          ForwardFrom;
		TGBOT_Chat*                          ForwardFromChat;
		uint64_t                             ForwardFromMessageId;
		SMAnsiString                         ForwardSignature;
		SMAnsiString                         ForwardSenderName;
		time_t                               ForwardDate;
		bool                                 IsTopicMessage;
		bool                                 IsAutomaticForward;
		TGBOT_Message*                       ReplyToMessage;
		TGBOT_User*                          ViaBot;
		time_t                               EditDate;
		bool                                 HasProtectedContent;
		SMAnsiString                         MediaGroupId;
		SMAnsiString                         AuthorSignature;
		SMAnsiString                         Text;
		TGBOT_ARRAY(TGBOT_MessageEntity)     Entities;
		TGBOT_Animation*                     Animation;
		TGBOT_Audio*                         Audio;
		TGBOT_Document*                      Document;
		TGBOT_ARRAY(TGBOT_PhotoSize)         Photo;
		TGBOT_Sticker*                       Sticker;
		TGBOT_Video*                         Video;
		TGBOT_VideoNote*                     VideoNote;
		TGBOT_Voice*                         Voice;
		SMAnsiString                         Caption;
		TGBOT_ARRAY(TGBOT_MessageEntity)     CaptionEntities;
		TGBOT_Contact*                       Contact;
		TGBOT_Dice*                          Dice;
		TGBOT_Game*                          Game;
		TGBOT_Poll*                          Poll;
		TGBOT_Venue*                         Venue;
		TGBOT_Location*                      Location;
		TGBOT_ARRAY(TGBOT_User)              NewChatMembers;
		TGBOT_User*                          LeftChatMember;
		SMAnsiString                         NewChatTitle;
		TGBOT_ARRAY(TGBOT_PhotoSize)         NewChatPhoto;
		bool                                 DeleteChatPhoto;
		bool                                 GroupChatCreated;
		bool                                 SupergroupChatCreated;
		bool                                 ChannelChatCreated;
		TGBOT_MessageAutoDeleteTimerChanged* MessageAutoDeleteTimerChanged;
		uint64_t                             MigrateToChatId;
		uint64_t                             MigrateFromChatId;
		TGBOT_Message*                       PinnedMessage;
		TGBOT_Invoice*                       Invoice;
		TGBOT_SuccessfulPayment*             SuccessfulPayment;
		TGBOT_PassportData*                  PassportData;
		TGBOT_ProximityAlertTriggered*       ProximityAlertTriggered;
		TGBOT_ForumTopicCreated*             ForumTopicCreated;
		TGBOT_ForumTopicClosed*              ForumTopicClosed;
		TGBOT_ForumTopicReopened*            ForumTopicReopened;
		TGBOT_VideoChatScheduled*            VideoChatScheduled;
		TGBOT_VideoChatStarted*              VideoChatStarted;
		TGBOT_VideoChatEnded*                VideoChatEnded;
		TGBOT_VideoChatParticipantsInvited*  VideoChatParticipantsInvited;
		TGBOT_WebAppData*                    WebAppData;
		TGBOT_InlineKeyboardMarkup*          ReplyMarkup;

		ADD_EXTERNAL_CONSTRUCTORS(TGBOT_Message)
		ADD_EXTERNAL_DESTRUCTORS(TGBOT_Message)

		virtual void InitAll() override;
		virtual void FreeAll() override;
		
		virtual void FromJSON(json_t *ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "message_id",                        MessageId                    );
			GetValueFromJSON(ObjectEntry, "message_thread_id",                 MessageThreadId              );
			GetValueFromJSON(ObjectEntry, "from",                              From                         );
			GetValueFromJSON(ObjectEntry, "sender_chat",                       SenderChat                   );
			GetValueFromJSON(ObjectEntry, "date",                              Date                         );
			GetValueFromJSON(ObjectEntry, "chat",                              Chat                         );
			GetValueFromJSON(ObjectEntry, "forward_from",                      ForwardFrom                  );
			GetValueFromJSON(ObjectEntry, "forward_from_chat",                 ForwardFromChat              );
			GetValueFromJSON(ObjectEntry, "forward_from_message_id",           ForwardFromMessageId         );
			GetValueFromJSON(ObjectEntry, "forward_signature",                 ForwardSignature             );
			GetValueFromJSON(ObjectEntry, "forward_sender_name",               ForwardSenderName            );
			GetValueFromJSON(ObjectEntry, "forward_date",                      ForwardDate                  );
			GetValueFromJSON(ObjectEntry, "is_topic_message",                  IsTopicMessage               );
			GetValueFromJSON(ObjectEntry, "is_automatic_forward",              IsAutomaticForward           );
			GetValueFromJSON(ObjectEntry, "reply_to_message",                  ReplyToMessage               );
			GetValueFromJSON(ObjectEntry, "via_bot",                           ViaBot                       );
			GetValueFromJSON(ObjectEntry, "edit_date",                         EditDate                     );
			GetValueFromJSON(ObjectEntry, "has_protected_content",             HasProtectedContent          );
			GetValueFromJSON(ObjectEntry, "media_group_id",                    MediaGroupId                 );
			GetValueFromJSON(ObjectEntry, "author_signature",                  AuthorSignature              );
			GetValueFromJSON(ObjectEntry, "text",                              Text                         );
			GetArrayFromJSON(ObjectEntry, "entities",                          Entities                     );
			GetValueFromJSON(ObjectEntry, "animation",                         Animation                    );
			GetValueFromJSON(ObjectEntry, "audio",                             Audio                        );
			GetValueFromJSON(ObjectEntry, "document",                          Document                     );
			GetArrayFromJSON(ObjectEntry, "photo",                             Photo                        );
			GetValueFromJSON(ObjectEntry, "sticker",                           Sticker                      );
			GetValueFromJSON(ObjectEntry, "video",                             Video                        );
			GetValueFromJSON(ObjectEntry, "video_note",                        VideoNote                    );
			GetValueFromJSON(ObjectEntry, "voice",                             Voice                        );
			GetValueFromJSON(ObjectEntry, "caption",                           Caption                      );
			GetArrayFromJSON(ObjectEntry, "caption_entities",                  CaptionEntities              );
			GetValueFromJSON(ObjectEntry, "contact",                           Contact                      );
			GetValueFromJSON(ObjectEntry, "dice",                              Dice                         );
			GetValueFromJSON(ObjectEntry, "game",                              Game                         );
			GetValueFromJSON(ObjectEntry, "poll",                              Poll                         );
			GetValueFromJSON(ObjectEntry, "venue",                             Venue                        );
			GetValueFromJSON(ObjectEntry, "location",                          Location                     );
			GetArrayFromJSON(ObjectEntry, "new_chat_members",                  NewChatMembers               );
			GetValueFromJSON(ObjectEntry, "left_chat_member",                  LeftChatMember               );
			GetValueFromJSON(ObjectEntry, "new_chat_title",                    NewChatTitle                 );
			GetArrayFromJSON(ObjectEntry, "new_chat_photo",                    NewChatPhoto                 );
			GetValueFromJSON(ObjectEntry, "delete_chat_photo",                 DeleteChatPhoto              );
			GetValueFromJSON(ObjectEntry, "group_chat_created",                GroupChatCreated             );
			GetValueFromJSON(ObjectEntry, "supergroup_chat_created",           SupergroupChatCreated        );
			GetValueFromJSON(ObjectEntry, "channel_chat_created",              ChannelChatCreated           );
			GetValueFromJSON(ObjectEntry, "message_auto_delete_timer_changed", MessageAutoDeleteTimerChanged);
			GetValueFromJSON(ObjectEntry, "migrate_to_chat_id",                MigrateToChatId              );
			GetValueFromJSON(ObjectEntry, "migrate_from_chat_id",              MigrateFromChatId            );
			GetValueFromJSON(ObjectEntry, "pinned_message",                    PinnedMessage                );
			GetValueFromJSON(ObjectEntry, "invoice",                           Invoice                      );
			GetValueFromJSON(ObjectEntry, "successful_payment",                SuccessfulPayment            );
			GetValueFromJSON(ObjectEntry, "passport_data",                     PassportData                 );
			GetValueFromJSON(ObjectEntry, "proximity_alert_triggered",         ProximityAlertTriggered      );
			GetValueFromJSON(ObjectEntry, "forum_topic_created",               ForumTopicCreated            );
			GetValueFromJSON(ObjectEntry, "forum_topic_closed",                ForumTopicClosed             );
			GetValueFromJSON(ObjectEntry, "forum_topic_reopened",              ForumTopicReopened           );
			GetValueFromJSON(ObjectEntry, "video_chat_scheduled",              VideoChatScheduled           );
			GetValueFromJSON(ObjectEntry, "video_chat_started",                VideoChatStarted             );
			GetValueFromJSON(ObjectEntry, "video_chat_ended",                  VideoChatEnded               );
			GetValueFromJSON(ObjectEntry, "video_chat_participants_invited",   VideoChatParticipantsInvited );
			GetValueFromJSON(ObjectEntry, "web_app_data",                      WebAppData                   );
			GetValueFromJSON(ObjectEntry, "reply_markup",                      ReplyMarkup                  );
		}
		virtual SMAnsiString ToJSON() const override
		{
			SMAnsiString ostream;
			PutValueToJSON(ostream, "message_id",                        MessageId                    );
			PutValueToJSON(ostream, "message_thread_id",                 MessageThreadId              );
			PutValueToJSON(ostream, "from",                              From                         );
			PutValueToJSON(ostream, "sender_chat",                       SenderChat                   );
			PutValueToJSON(ostream, "date",                              Date                         );
			PutValueToJSON(ostream, "chat",                              Chat                         );
			PutValueToJSON(ostream, "forward_from",                      ForwardFrom                  );
			PutValueToJSON(ostream, "forward_from_chat",                 ForwardFromChat              );
			PutValueToJSON(ostream, "forward_from_message_id",           ForwardFromMessageId         );
			PutValueToJSON(ostream, "forward_signature",                 ForwardSignature             );
			PutValueToJSON(ostream, "forward_sender_name",               ForwardSenderName            );
			PutValueToJSON(ostream, "forward_date",                      ForwardDate                  );
			PutValueToJSON(ostream, "is_topic_message",                  IsTopicMessage               );
			PutValueToJSON(ostream, "is_automatic_forward",              IsAutomaticForward           );
			PutValueToJSON(ostream, "reply_to_message",                  ReplyToMessage               );
			PutValueToJSON(ostream, "via_bot",                           ViaBot                       );
			PutValueToJSON(ostream, "edit_date",                         EditDate                     );
			PutValueToJSON(ostream, "has_protected_content",             HasProtectedContent          );
			PutValueToJSON(ostream, "media_group_id",                    MediaGroupId                 );
			PutValueToJSON(ostream, "author_signature",                  AuthorSignature              );
			PutValueToJSON(ostream, "text",                              Text                         );
			PutArrayToJSON(ostream, "entities",                          Entities                     );
			PutValueToJSON(ostream, "animation",                         Animation                    );
			PutValueToJSON(ostream, "audio",                             Audio                        );
			PutValueToJSON(ostream, "document",                          Document                     );
			PutArrayToJSON(ostream, "photo",                             Photo                        );
			PutValueToJSON(ostream, "sticker",                           Sticker                      );
			PutValueToJSON(ostream, "video",                             Video                        );
			PutValueToJSON(ostream, "video_note",                        VideoNote                    );
			PutValueToJSON(ostream, "voice",                             Voice                        );
			PutValueToJSON(ostream, "caption",                           Caption                      );
			PutArrayToJSON(ostream, "caption_entities",                  CaptionEntities              );
			PutValueToJSON(ostream, "contact",                           Contact                      );
			PutValueToJSON(ostream, "dice",                              Dice                         );
			PutValueToJSON(ostream, "game",                              Game                         );
			PutValueToJSON(ostream, "poll",                              Poll                         );
			PutValueToJSON(ostream, "venue",                             Venue                        );
			PutValueToJSON(ostream, "location",                          Location                     );
			PutArrayToJSON(ostream, "new_chat_members",                  NewChatMembers               );
			PutValueToJSON(ostream, "left_chat_member",                  LeftChatMember               );
			PutValueToJSON(ostream, "new_chat_title",                    NewChatTitle                 );
			PutArrayToJSON(ostream, "new_chat_photo",                    NewChatPhoto                 );
			PutValueToJSON(ostream, "delete_chat_photo",                 DeleteChatPhoto              );
			PutValueToJSON(ostream, "group_chat_created",                GroupChatCreated             );
			PutValueToJSON(ostream, "supergroup_chat_created",           SupergroupChatCreated        );
			PutValueToJSON(ostream, "channel_chat_created",              ChannelChatCreated           );
			PutValueToJSON(ostream, "message_auto_delete_timer_changed", MessageAutoDeleteTimerChanged);
			PutValueToJSON(ostream, "migrate_to_chat_id",                MigrateToChatId              );
			PutValueToJSON(ostream, "migrate_from_chat_id",              MigrateFromChatId            );
			PutValueToJSON(ostream, "pinned_message",                    PinnedMessage                );
			PutValueToJSON(ostream, "invoice",                           Invoice                      );
			PutValueToJSON(ostream, "successful_payment",                SuccessfulPayment            );
			PutValueToJSON(ostream, "passport_data",                     PassportData                 );
			PutValueToJSON(ostream, "proximity_alert_triggered",         ProximityAlertTriggered      );
			PutValueToJSON(ostream, "forum_topic_created",               ForumTopicCreated            );
			PutValueToJSON(ostream, "forum_topic_closed",                ForumTopicClosed             );
			PutValueToJSON(ostream, "forum_topic_reopened",              ForumTopicReopened           );
			PutValueToJSON(ostream, "video_chat_scheduled",              VideoChatScheduled           );
			PutValueToJSON(ostream, "video_chat_started",                VideoChatStarted             );
			PutValueToJSON(ostream, "video_chat_ended",                  VideoChatEnded               );
			PutValueToJSON(ostream, "video_chat_participants_invited",   VideoChatParticipantsInvited );
			PutValueToJSON(ostream, "web_app_data",                      WebAppData                   );
			PutValueToJSON(ostream, "reply_markup",                      ReplyMarkup                  );
			return std::move(ostream);
		}
};

class TGBOT_CallbackQuery : public TGBOT_API_Class
{
	public:
		SMAnsiString  Id;
		TGBOT_User    *From;
		TGBOT_Message *Message;
		SMAnsiString  InlineMessageId,
		              ChatInstance,
		              Data,
		              GameShortName;

		ADD_CONSTRUCTORS(TGBOT_CallbackQuery)
		ADD_DESTRUCTORS(TGBOT_CallbackQuery)

		virtual void InitAll() override
		{
			CLEAR_STR(Id);
			RESET_PTR(From);
			RESET_PTR(Message);
			CLEAR_STR(InlineMessageId);
			CLEAR_STR(ChatInstance);
			CLEAR_STR(Data);
			CLEAR_STR(GameShortName);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(this->From);
			DELETE_SINGLE_OBJECT(this->Message);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "id",                Id);
			GetValueFromJSON(ObjectEntry, "from",              From);
			GetValueFromJSON(ObjectEntry, "message",           Message);
			GetValueFromJSON(ObjectEntry, "inline_message_id", InlineMessageId);
			GetValueFromJSON(ObjectEntry, "chat_instance",     ChatInstance);
			GetValueFromJSON(ObjectEntry, "data",              Data);
			GetValueFromJSON(ObjectEntry, "game_short_name",   GameShortName);
		}
		virtual SMAnsiString ToJSON() const override
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

class TGBOT_Update : public TGBOT_API_Class
{
	public:
		uint64_t            UpdateId;
		TGBOT_Message       *Message,
		                    *EditedMessage,
		                    *ChannelPost,
		                    *EditedChannelPost;
		TGBOT_CallbackQuery *CallbackQuery;

		ADD_CONSTRUCTORS(TGBOT_Update)
		ADD_DESTRUCTORS(TGBOT_Update)

		virtual void InitAll() override
		{
			RESET_ULONGLONG(UpdateId);
			RESET_PTR(Message);
			RESET_PTR(EditedMessage);
			RESET_PTR(ChannelPost);
			RESET_PTR(EditedChannelPost);
			RESET_PTR(CallbackQuery);
		}
		virtual void FreeAll() override
		{
			DELETE_SINGLE_OBJECT(this->Message);
			DELETE_SINGLE_OBJECT(this->EditedMessage);
			DELETE_SINGLE_OBJECT(this->ChannelPost);
			DELETE_SINGLE_OBJECT(this->EditedChannelPost);
			DELETE_SINGLE_OBJECT(this->CallbackQuery);
		}

		virtual void FromJSON(json_t* ObjectEntry) override
		{
			FreeAll();
			InitAll();
			GetValueFromJSON(ObjectEntry, "update_id",           UpdateId         );
			GetValueFromJSON(ObjectEntry, "message",             Message          );
			GetValueFromJSON(ObjectEntry, "edited_message",      EditedMessage    );
			GetValueFromJSON(ObjectEntry, "channel_post",        ChannelPost      );
			GetValueFromJSON(ObjectEntry, "edited_channel_post", EditedChannelPost);
			GetValueFromJSON(ObjectEntry, "callback_query",      CallbackQuery    );
		}
		virtual SMAnsiString ToJSON() const override
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

using TGBOTInlineKeyboardMarkupPtr = std::unique_ptr<TGBOT_InlineKeyboardMarkup>;

bool tgbot_GetUpdates(std::vector<TGBOT_Update*>& updates, uint64_t offset = 0);
uint64_t GetChatIDFromUpdate(TGBOT_Update* Upd);
TGBOT_User* GetUserFromUpdate(TGBOT_Update* Upd);
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