#include "tg_bot.h"

/*template<class type>
class supervector : public std::vector<type*>
{
	public:
		~supervector()
		{
			const size_t count = this->size();
			for (size_t i = 0; i < count; i++)
			{
				type *x = this->at(i);
				delete x;
			}
		}
};*/

#ifdef _WIN32
void usleep(unsigned int usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (__int64)usec);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#endif

SMAnsiString ExtractFilePath(SMAnsiString FileName)
{
	for(int i=FileName.length()-1; i>=0; i--)
	{
		if(FileName[i] == '/')
		{
			return FileName.Delete(i+1, FileName.length());
		}
	}
	
	return "";
}

bool LoadLastBotState(SMAnsiString FileName)
{
	std::ifstream file;
	StringList    file_strings;
	unsigned int  file_size;    // размер файла
	struct stat   st;           // структура для получения размера файла
	char          *fbuffer;     // буфер, куда читаем
	int           size_of_char;
	char          letter;
	int           write_pos;    // позиция в буфере, куда запишем считанный символ
	
	// размер файла
	stat(C_STR(FileName), &st);
	file_size = st.st_size;
	
	if(file_size == 0) 
	{
		SetCurrentUpdate(0);
		return false;
	}
	
	// открываем файл
	file.open(C_STR(FileName), std::ios::binary);
	if(!file.is_open())
	{
		SetCurrentUpdate(0);
		return false;
	}
	
	// читаем файл
	fbuffer = new char[file_size+1];
	size_of_char = sizeof(letter);
	write_pos = 0;
	for(int i=0; i<file_size; i++)
	{
		file.read(reinterpret_cast<char*>(&letter), size_of_char);
		if(letter != '\n') 
		{
			if(i != (file_size-1))
			{
				fbuffer[write_pos] = letter;
				write_pos++;
			}
			else
			{
				fbuffer[write_pos] = '\0';
				file_strings.push_back(fbuffer);
			}
		}
		else
		{
			fbuffer[write_pos] = '\0';
			file_strings.push_back(fbuffer);
			write_pos = 0;
		}
	}
	file.close();
	delete[] fbuffer;
	
	// присваиваем параметры
	SetCurrentUpdate(atoi(C_STR(file_strings[0])));
	
	return true;
}

bool SaveLastBotState(SMAnsiString FileName)
{
	std::ofstream	file;
	
	file.open(C_STR(FileName));
	if(!file.is_open()) return false;
	
	file << GetCurrentUpdate() << "\n";

	file.close();
	return true;
}

static SMAnsiString GetFormatOutputStr(const SMAnsiString& Message, int Color)
{
	// Color:
	// 0 - default
	// 1 - yellow
	// 2 - green
	// 3 - red
	// 4 - lightblue
	// 5 - purpure
	// 6 - white
	SMDateTime 		current_time;
	SMAnsiString	begin_format,
					end_format;

	switch (Color)
	{
		#define WMCOLOR(color, begin_fmt, end_fmt) { case color: { begin_format = begin_fmt; end_format = end_fmt; break; } }
		WMCOLOR(TGB_TEXTCOLOR_DEFAULT, "", "");
		WMCOLOR(TGB_TEXTCOLOR_YELLOW, "\033[0;33m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_GREEN, "\033[0;32m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_RED, "\033[0;31m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_LIGHTBLUE, "\033[0;36m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_PURPURE, "\033[0;35m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_WHITE, "\033[0;37m", "\033[0m");
		WMCOLOR(TGB_TEXTCOLOR_BLUE, "\033[0;34m", "\033[0m");
		default: { begin_format = ""; end_format = ""; }
		#undef WMCOLOR
	}
	return SMAnsiString().smprintf("[%s] %s%s%s\n", C_STR(current_time.GMDateTimeString()), C_STR(begin_format), C_STR(Message), C_STR(end_format));
}
static SMAnsiString GetFormatStr(const SMAnsiString& Message)
{
	SMDateTime 		current_time;
	return SMAnsiString::smprintf("[%s] %s\n", C_STR(current_time.GMDateTimeString()), C_STR(Message));
}

void WriteMessage(const SMAnsiString &Message, int Color)
{
	static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&console_mutex);
	std::cout << GetFormatOutputStr(Message, Color);
	pthread_mutex_unlock(&console_mutex);
}

void WriteFormatMessage(const SMAnsiString& Format, int Color, ...)
{
	static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;

	va_list ap;
	va_start(ap, Color);
	pthread_mutex_lock(&console_mutex);
	vprintf(C_STR(GetFormatOutputStr(Format, Color)), ap);
	pthread_mutex_unlock(&console_mutex);
	va_end(ap);
}

void AppendStringToLog(SMAnsiString filename, const SMAnsiString& Message)
{
	if (!filename.IsEmpty())
	{
		std::ofstream outf(filename, std::ios::binary | std::ios::app);
		outf.write(Message, Message.length());
		outf.close();
	}
}

StringList* LoadTextFromFile(SMAnsiString FileName)
{
	// размер файла
	struct stat st;
	stat(C_STR(FileName), &st);
	off_t file_size = st.st_size;
	
	// открываем файл
	std::ifstream file;
	file.open(C_STR(FileName), std::ios::binary);
	if(!file.is_open()) return NULL;
	
	StringList *text = new StringList;
	if(file_size == 0) return text;
	
	char *fbuffer = new char[file_size];
	char letter;
	size_t size_of_char = sizeof(letter);
	size_t write_pos = 0;
	for(int i=0; i<file_size; i++)
	{
		file.read(reinterpret_cast<char*>(&letter), size_of_char);
		if(letter != '\n') 
		{
			if (letter != '\r')
			{
				if (i != (file_size - 1))
					fbuffer[write_pos++] = letter;
				else
				{
					fbuffer[write_pos] = '\0';
					text->push_back(fbuffer);
				}
			}
			else continue;
		}
		else
		{
			fbuffer[write_pos] = '\0';
			text->push_back(fbuffer);
			write_pos = 0;
		}
	}
	
	DELETE_ARRAY_OBJECT(fbuffer);
	return text;
}

bool LoadTextFromFile_v2(SMAnsiString FileName, StringList *Output)
{
	Output->clear();
	
	// размер файла
	struct stat   st;
	stat(C_STR(FileName), &st);
	off_t file_size = st.st_size;
	
	// открываем файл
	std::ifstream file;
	file.open(C_STR(FileName), std::ios::binary);
	if(!file.is_open()) return false;
	if(file_size == 0) return true;
	
	char *fbuffer = new char[file_size];
	char letter;
	size_t size_of_char = sizeof(letter);
	size_t write_pos = 0;
	for(off_t i=0; i<file_size; i++)
	{
		file.read(reinterpret_cast<char*>(&letter), size_of_char);
		if(letter != '\n') 
		{
			if(i != (file_size-1))
				fbuffer[write_pos++] = letter;
			else
			{
				fbuffer[write_pos] = '\0';
				Output->push_back(fbuffer);
			}
		}
		else
		{
			fbuffer[write_pos] = '\0';
			Output->push_back(fbuffer);
			write_pos = 0;
		}
	}
	
	DELETE_ARRAY_OBJECT(fbuffer);
	return true;
}

SMAnsiString TimeDifferenceString(time_t Time1, time_t Time2)
{
	time_t	Difference = Time1 - Time2;

	if(Difference < 60) return SMAnsiString(Difference) + " сек.";
	if(Difference < 3600) return SMAnsiString(Difference/60) + " мин.";
	if(Difference < 86400) return SMAnsiString(Difference/3600) + " ч.";
    if(Difference < 2592000) return SMAnsiString(Difference/86400) + " дн.";
	else return SMAnsiString(Difference/86400) + " дн.";
}

void Hexlify(char* hex, const unsigned char* bin, int len)
{
	hex[0] = 0;
	for (int i = 0; i < len; i++)
		sprintf(hex + strlen(hex), "%02x", bin[i]);
}

SMAnsiString ParamsToJSON(const StringList &keys, const StringList &values)
{
	const size_t keys_size = keys.size();
	const size_t values_size = values.size();

	if (keys_size != values_size) return "{}";

	SMAnsiString ret = "{";
	for (size_t i = 0; i < keys_size; i++)
	{
		ret += SMAnsiString((i) ? "," : ""); 
		ret += SMAnsiString::smprintf("\"%s\":\"%s\"", C_STR(keys.at(i)), C_STR(values.at(i)));
	}
	ret += "}";
	return ret;
}

void ParamsFromJSON(const SMAnsiString &jsonstr, const StringList &keys, StringList &values)
{
	values.clear();
	if (jsonstr.IsEmpty()) return;

	json_error_t error;
	json_t *root = json_loads(C_STR(jsonstr), 0, &error);
	if (!root) return;

	json_t* jobj;
	const size_t keys_size = keys.size();
	for (size_t i = 0; i < keys_size; i++)
	{
		jobj = json_object_get(root, C_STR(keys.at(i)));
		if (jobj)
			values.push_back(json_string_value(jobj));
		else
			values.push_back("");
	}
	json_decref(root);
}

SMAnsiString MakeKeyListStr(const SMKeyList& list)
{
	const size_t list_size = list.size();
	SMAnsiString ret;
	for (size_t i = 0; i < list_size; i++)
	{
		ret += SMAnsiString((i) ? " OR " : "");
		ret += SMAnsiString::smprintf("id=%llu", list.at(i));
	}
	return ret;
}

// конвертировать день недели (в результате будет 0 - пн, 1 - вт и т.д.)
int DayOfWeekCovert(int Src)
{
	switch (Src)
	{
		case 0:
		{
			return 6;
		}
		default: return Src - 1;
	}
}
// определить день недели даты DD.MM.YYYY
int WhatDay(int D, int M, int Y)
{
	int a = (14 - M) / 12;
	int y = Y - a;
	int m = M + 12 * a - 2;

	return DayOfWeekCovert((7000 + (D + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12)) % 7);
}
// определение количества дней в месяце
int GetDays(int month, int year) // months - 0 for january, ..., 11 for december
{
	static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int result = -1;

	if (month < 0 || month > 11)
		return result;

	result = days[month];
	if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
		if (month == 1)
			++result;

	return result;
}

bool GetDateFromString(const SMAnsiString& str, int &day, int &month, int &year)
{
	day = month = year = 0;
	// сколько точек в дате? правильно - 2!
	int counter = 0;
	const int _strlen = str.length();
	SMAnsiString c[3];
	char buf[8] = { '\0' };
	char* bp=buf;
	for (int i = 0; i < _strlen; i++)
	{
		if (str[i] == '.')
		{
			*bp = '\0';
			c[counter] = buf;
			if (!c[counter].IsValidNumber()) 
				return false;
			counter++;
			bp = buf;
			memset(buf, 0, SizeOfArray(buf));
		}
		else if (str[i] == ' ') 
			return false;
		else
		{
			*bp = str[i]; 
			bp++;
		}	

		if (i == _strlen - 1)
		{
			*bp = '\0';
			c[counter] = buf;
			if (!c[counter].IsValidNumber()) 
				return false;
		}
	}
	if (counter != 2) return false;

	day = c[0];
	month = c[1];
	year = c[2];

	if (day < 1 || day > 31) return false;
	if (month < 1 || month > 12) return false;
	if (year < 1970 || year > 2038) return false;

	return true;
}

SMAnsiString MakePermissionString(uint64_t permission, const StringList *str)
{
	SMAnsiString msg;
	const size_t numstr = str ? str->size() : 0ull;
	if (numstr)
	{
		msg += "\n\n<b>Расшифровка:</b>\n";
		for (size_t i=0; i<numstr; i++)
		{
			if (i) msg += "\n";
			msg += (GetBit64(permission, i)) ? "✅ " : "⛔️ ";
			msg += str->at(i);
		}
	}
	return SMAnsiString::smprintf
	(
		"<b>dec:</b> <code>%llu</code>\n"
		"<b>hex:</b> <code>%#010x</code>\n"
		"<b>bin:</b> <code>%s</code>"
		"%s",
		permission,
		permission,
		C_STR(DecToBin<uint64_t>(permission)),
		C_STR(msg)
	);
}

// MEM.CPP
template <size_t _sz, size_t _alignment>
class SMAllocator
{
	protected:
		void *mem;
		size_t sz;
		size_t alignment;

		uint8_t* allocmask;
		size_t numblock;

	public:
		SMAllocator() : sz(_sz), alignment(_alignment)
		{
			mem = malloc(sz);
			numblock = _sz / _alignment;
			allocmask = new uint8_t[numblock];
		}
		~SMAllocator()
		{
			free(mem);
			delete[] allocmask;
		}
};

/*void* operator new(size_t size)
{
	return malloc(size);
}*/
