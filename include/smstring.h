#ifndef SM_STRINGCLASSES
#define SM_STRINGCLASSES

#pragma intrinsic(strlen)
#pragma intrinsic(memcpy)

#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>

#define STR_ALIGNMENT 256ULL // По-умолчанию под строку будет аллоцироваться STR_ALIGNMENT байт. Если для выполнения какой-либо операции
                             // понадобится больше места, сделаем переаллокацию.

#define MSTRLEN(_str)  (int)strlen(_str)
#define C_STR(str)     str.c_str()
#define CLEAR_STR(str) str[0]='\0'

inline int IsStrEmpty(const char* p) { return !(p && *p); } 

// строки выровнены по STR_ALIGNMENT байт. Т.е. их bufferSize всегда кратный STR_ALIGNMENT
class SMAnsiString
{
	private:
		char  *data       = nullptr;       // основной буфер
		size_t bufferSize = STR_ALIGNMENT; // длина основного буфера

		inline void Alloc(size_t len)
		{
			const size_t newsz = AlignedSize(len, STR_ALIGNMENT);
			data = new char[newsz];
			bufferSize = newsz;
		}
		inline void Reallocate(size_t newlen)
		{
			if (newlen < bufferSize) return;
			char* newdata = new char[newlen];
			if (data)
			{
				memcpy(newdata, data, bufferSize);
				delete[] data;
			}
			bufferSize = newlen;
			data = newdata;
		}
		inline void ReallocateIfNeeded(size_t slen, bool addfl=false)
		{
			const size_t bsz = (addfl) ? strlen(data)+slen+1ull : slen+1ull;
			if (bufferSize < bsz)
				Reallocate(AlignedSize(bsz, STR_ALIGNMENT));
		}

	public:
		// конструкторы
		SMAnsiString()
		{ 
			data = new char[bufferSize];
			data[0] = '\0';
		}
		SMAnsiString(const char *str) 
		{
			const size_t slen = (str)?strlen(str):0ull;
			Alloc(slen);
			if(slen) memcpy(data, str, slen);
			data[slen] = '\0';
		}
		SMAnsiString(const char* str, size_t len, int dummy)
		{
			const size_t slen = (str) ? len : 0ull;
			Alloc(slen + 1);
			if (slen) memcpy(data, str, slen);
			data[slen] = '\0';
		}
		SMAnsiString(char* str, size_t len) // сотрём сами
		{
			data = str;
			bufferSize = AlignedSize(len, STR_ALIGNMENT);
		}
		SMAnsiString(char value) { data = new char[bufferSize]; data[0]=value; data[1]='\0'; }
#ifndef __GNUG__
		SMAnsiString(int value) { sprintf_s(data = new char[bufferSize], bufferSize, "%d\0", value); }
		SMAnsiString(long value) { sprintf_s(data = new char[bufferSize], bufferSize, "%ld\0", value); }
		SMAnsiString(long long value) { sprintf_s(data = new char[bufferSize], bufferSize, "%lld\0", value); }
		SMAnsiString(unsigned int value) { sprintf_s(data = new char[bufferSize], bufferSize, "%u\0", value); }
		SMAnsiString(unsigned long value) { sprintf_s(data = new char[bufferSize], bufferSize, "%lu\0", value); }
		SMAnsiString(unsigned long long value) { sprintf_s(data = new char[bufferSize], bufferSize, "%llu\0", value); }
#else
		SMAnsiString(int value) { sprintf(data = new char[bufferSize], "%d", value); }
		SMAnsiString(long value) { sprintf(data = new char[bufferSize], "%ld", value); }
		SMAnsiString(long long value) { sprintf(data = new char[bufferSize], "%lld", value); }
		SMAnsiString(unsigned int value) { sprintf(data = new char[bufferSize], "%u", value); }
		SMAnsiString(unsigned long value) { sprintf(data = new char[bufferSize], "%lu", value); }
		SMAnsiString(unsigned long long value) { sprintf(data = new char[bufferSize], "%llu", value); }
#endif
		SMAnsiString(double value)
		{ 
			sprintf(data = new char[bufferSize], "%f", value);
			const int stlen = MSTRLEN(data);
			for (int i = stlen-1, f=0; i >= 0; i--)
			{
				if (data[i] == '0') f=1;
				else if (data[i] == '.') { data[i+2] = '\0'; break; }
				else { if (f) data[i + 1] = '\0'; break; }
			}
		}
		SMAnsiString(bool value)
		{
			const char* str = (value) ? "true" : "false";
			size_t slen = strlen(str);
			data = new char[bufferSize];
			memcpy(data, str, slen);
			data[slen] = '\0';
		}

		SMAnsiString(const SMAnsiString& value)
		{
			size_t bsz = strlen(value.c_str()) + 1;
			bufferSize = value.bufferSize;
			data = new char[bufferSize];
			memcpy(data, value.c_str(), bsz);
		}
		SMAnsiString(SMAnsiString&& rvalue) noexcept
		{
			data = rvalue.data;
			bufferSize = rvalue.bufferSize;
			rvalue.data = nullptr;
			rvalue.bufferSize = 0ull;
		}

		// деструктор
		~SMAnsiString()
		{
			if(data) delete[] data;
		}

		// выравнивание на align
		static inline size_t AlignedSize(size_t val, size_t align)
		{
			return val + (align - (val % align));
		}

		// методы
		const char* c_str() const { return data; }      // возвращаем const buffer со строкой
		inline size_t length() const { return strlen(data); }  // длина строки
		inline size_t SetChar(size_t idx, const char c)
		{
			const size_t sl = strlen(data);
			if (idx < 0) idx = 0;
			else if (idx >= sl) idx = sl-1ull;
			data[idx] = c;
			return idx;
		}
		SMAnsiString GetLowerCase()
		{
			SMAnsiString Ret(data);
			const size_t rl = strlen(data);

			for (size_t i = 0; i < rl; i++)
			{
				if (((int)Ret[i] >= 65) && ((int)Ret[i] <= 90))
					Ret.SetChar(i, (int)Ret[i] + 32);
			}

			return std::move(Ret);
		}
		SMAnsiString& ToLowerCase()
		{
			size_t slen = strlen(data);
			for (size_t i = 0; i < slen; i++)
			{
				if (((int)data[i] >= 65) && ((int)data[i] <= 90))
					data[i] = (int)data[i] + 32;
			}
			return *this;
		}
		int Pos(const char c) const
		{
			const int sldata = MSTRLEN(data);
			for (int i = 0; i < sldata; i++)
			{
				if (data[i] == c) return i;
			}
			return -1;
		}
		int PosSS(const SMAnsiString &substring) const
		{
			const int sslen = MSTRLEN(substring.data);
			const int sldata = MSTRLEN(data);
			const char* strss = substring.c_str();

			if (sslen > sldata) return -1;
			for (int i = 0; i < sldata - sslen + 1; i++)
			{
				for (int j = 0; j < sslen; j++)
				{
					if (data[i + j] != strss[j]) break;
					else if (j == (sslen - 1)) return i;
				}
			}
			return -1;
		}
		SMAnsiString Delete(size_t Index, size_t Count) const
		{
			SMAnsiString ret(data);
			const size_t sl = strlen(data);
			if ((Count > 0ull) && (Index < sl))
			{
				const size_t r_ind = ((Index + Count) > sl) ? sl : (Index + Count);
				const size_t realLn = (sl - r_ind) + 1ull;
				memmove(&ret.data[Index], &ret.data[r_ind], realLn);
			}
			return std::move(ret);
		}
		SMAnsiString& DeleteMyself(size_t Index, size_t Count)
		{
			const size_t sl = strlen(data);
			if ((Count > 0) && (Index < sl))
			{
				const size_t r_ind = ((Index + Count) > sl) ? sl : (Index + Count);
				const size_t realLn = (sl - r_ind) + 1ull;
				memmove(&data[Index], &data[r_ind], realLn);
			}
			return *this;
		}
		static SMAnsiString smprintf(const char* format, ...)
		{
			if (!format) return "";
			size_t bsz = AlignedSize(strlen(format)+10240ull, STR_ALIGNMENT);
			va_list ap;
			va_start(ap, format);
			char *buf = new char[bsz];
			vsprintf(buf, format, ap);
			va_end(ap);
			SMAnsiString ret(buf, bsz);
			return std::move(ret);
		}
		void smprintf_s(const char* format, ...)
		{
			if (format)
			{
				size_t bsz = strlen(format) + 10240ull;
				ReallocateIfNeeded(bsz);
				va_list ap;
				va_start(ap, format);
				vsprintf(data, format, ap);
				va_end(ap);
			}
			else
				*data = '\0';
		}
		bool IsEmpty() const { return (strlen(data) <= 0); }
		bool IsValidNumber() const 
		{
			if (!strlen(data)) return false;
			char* ptr=data;
			while (*ptr != '\0')
			{
				if (!isdigit((unsigned char)(*(ptr++)))) return false;
			}
			return true;
		}

		// перегрузка операторов
		SMAnsiString& operator=(const SMAnsiString& value)
		{
			size_t bsz = strlen(value.c_str()) + 1;
			ReallocateIfNeeded(bsz);
			memcpy(data, value.c_str(), bsz);
			return *this;
		}
		SMAnsiString& operator=(SMAnsiString&& rvalue) noexcept
		{
			if(data) delete[] data;
			data = rvalue.data;
			bufferSize = rvalue.bufferSize;
			rvalue.data = nullptr;
			rvalue.bufferSize = 0ull;
			return *this;
		}
		SMAnsiString& operator=(const char* str)
		{
			size_t slen = (str)?strlen(str):0ull;
			if (slen)
			{
				ReallocateIfNeeded(slen);
				memcpy(data, str, slen);
			}
			if (!data) Alloc(1ull);
			data[slen] = '\0';
			return *this;
		}
		SMAnsiString& operator=(char value)
		{
			data[0] = value;
			data[1] = '\0';
			return *this;
		}

		SMAnsiString operator+(const SMAnsiString& Value) const
		{	
			SMAnsiString ret(data);
			ret += Value;
			return std::move(ret);
		}
		SMAnsiString operator+(const char *Value) const
		{
			SMAnsiString ret(data);
			ret += Value;
			return std::move(ret);
		}
		SMAnsiString operator+(char Value) const
		{
			SMAnsiString ret(data);
			ret += Value;
			return std::move(ret);
		}
		SMAnsiString& operator+=(const SMAnsiString& Value)
		{
			size_t dataLen = strlen(data);
			size_t valLen = Value.length();
			ReallocateIfNeeded(valLen, true);
			memcpy(&data[strlen(data)], Value.c_str(), valLen);
			data[dataLen + valLen] = '\0';
			return *this;
		}
		SMAnsiString& operator+=(const char* Value)
		{
			if (Value)
			{
				size_t dataLen = strlen(data);
				size_t valLen = strlen(Value);
				ReallocateIfNeeded(valLen, true);
				memcpy(&data[strlen(data)], Value, valLen);
				data[dataLen + valLen] = '\0';
			}
			return *this;
		}
		SMAnsiString& operator+=(char Value)
		{
			size_t dataLen = strlen(data);
			size_t valLen = 1ULL;
			ReallocateIfNeeded(valLen, true);
			memcpy(&data[strlen(data)], &Value, valLen);
			data[dataLen + valLen] = '\0';
			return *this;
		}

		bool operator==(const SMAnsiString& Value) const
		{
			const char* _value = Value.data;
			const size_t sl = strlen(data);
			const size_t vsl = strlen(_value);

			// сначала проверяем длину строк, если они разные - нет смысла дальше сравнивать
			if (sl != vsl) return false;
			// если обе строки пустые, то они равны
			if (sl == 0ull) return true;

			// за один такт можно сравнить сразу 8 байт
			const size_t steplen = sizeof(uint64_t);
			const size_t numiter = sl / steplen;
			const size_t ost = sl % steplen;
			const uint64_t * ptrL = reinterpret_cast<const uint64_t*>(_value);
			const uint64_t * ptrP = reinterpret_cast<const uint64_t*>(data);
			for (size_t i = 0; i < numiter; i++)
			{
				if (*ptrL != *ptrP)
					return false;
				ptrL++;
				ptrP++;
			}
			if (ost)
			{
				// остатки сравниваем посимвольно
				const char* cptrL = reinterpret_cast<const char*>(ptrL);
				const char* cptrP = reinterpret_cast<const char*>(ptrP);
				for (size_t i = 0; i < ost; i++)
				{
					if (*cptrL != *cptrP)
						return false;
					cptrL++;
					cptrP++;
				}
			}
			return true;
		}
		bool operator==(const char *Value) const
		{
			const size_t sl = strlen(data);
			const size_t vsl = (Value)?strlen(Value):0ull;

			// сначала проверяем длину строк, если они разные - нет смысла дальше сравнивать
			if (sl != vsl) return false;
			// если обе строки пустые, то они равны
			if (sl == 0ull) return true;

			// за один такт можно сравнить сразу 8 байт
			const size_t steplen = sizeof(uint64_t);
			const size_t numiter = sl / steplen;
			const size_t ost = sl % steplen;
			const uint64_t *ptrL = reinterpret_cast<const uint64_t*>(Value);
			const uint64_t *ptrP = reinterpret_cast<const uint64_t*>(data);
			for (size_t i = 0; i < numiter; i++)
			{
				if (*ptrL != *ptrP)
					return false;
				ptrL++;
				ptrP++;
			}
			if (ost)
			{
				// остатки сравниваем посимвольно
				const char *cptrL = reinterpret_cast<const char*>(ptrL);
				const char *cptrP = reinterpret_cast<const char*>(ptrP);
				for (size_t i = 0; i < ost; i++)
				{
					if (*cptrL != *cptrP)
						return false;
					cptrL++;
					cptrP++;
				}
			}
			return true;
		}

		bool operator!=(const SMAnsiString& Value) const
		{
			const char* _value = Value.data;
			const size_t sl = strlen(data);
			const size_t vsl = strlen(_value);
			

			// сначала проверяем длину строк, если они разные - нет смысла дальше сравнивать
			if (sl != vsl) return true;
			// если обе строки пустые, то они равны
			if (sl == 0ull) return false;

			// за один такт можно сравнить сразу 8 байт
			const size_t steplen = sizeof(uint64_t);
			const size_t numiter = sl / steplen;
			const size_t ost = sl % steplen;
			const uint64_t * ptrL = reinterpret_cast<const uint64_t*>(_value);
			const uint64_t * ptrP = reinterpret_cast<const uint64_t*>(data);
			for (size_t i = 0; i < numiter; i++)
			{
				if (*ptrL != *ptrP)
					return true;
				ptrL++;
				ptrP++;
			}
			if (ost)
			{
				// остатки сравниваем посимвольно
				const char* cptrL = reinterpret_cast<const char*>(ptrL);
				const char* cptrP = reinterpret_cast<const char*>(ptrP);
				for (size_t i = 0; i < ost; i++)
				{
					if (*cptrL != *cptrP)
						return true;
					cptrL++;
					cptrP++;
				}
			}
			return false;
		}
		bool operator!=(const char* Value) const
		{
			const size_t sl = strlen(data);
			const size_t vsl = (Value) ? strlen(Value) : 0ull;

			// сначала проверяем длину строк, если они разные - нет смысла дальше сравнивать
			if (sl != vsl) return true;
			// если обе строки пустые, то они равны
			if (sl == 0ull) return false;

			// за один такт можно сравнить сразу 8 байт
			const size_t steplen = sizeof(uint64_t);
			const size_t numiter = sl / steplen;
			const size_t ost = sl % steplen;
			const uint64_t *ptrL = reinterpret_cast<const uint64_t*>(Value);
			const uint64_t *ptrP = reinterpret_cast<const uint64_t*>(data);
			for (size_t i = 0; i < numiter; i++)
			{
				if (*ptrL != *ptrP)
					return true;
				ptrL++;
				ptrP++;
			}
			if (ost)
			{
				// остатки сравниваем посимвольно
				const char *cptrL = reinterpret_cast<const char*>(ptrL);
				const char *cptrP = reinterpret_cast<const char*>(ptrP);
				for (size_t i = 0; i < ost; i++)
				{
					if (*cptrL != *cptrP)
						return true;
					cptrL++;
					cptrP++;
				}
			}
			return false;
		}

		char& operator[](const size_t idx) const { return data[idx]; }
		
		operator int() const { return atoi(data); }
		operator long() const { return atol(data); }
		operator long long() const { return atoll(data); }
		operator unsigned char() const { return operator unsigned int(); }
		operator unsigned short() const { return operator unsigned int(); }
		operator unsigned int() const { return (unsigned int)atoll(data); }
		operator unsigned long() const { return strtoul(data, nullptr, 10); }
		operator unsigned long long() const { return strtoull(data, nullptr, 10); }
		operator const char* () const { return data; }
		operator bool() const { return (!strlen(data)) || (SMAnsiString(data).GetLowerCase() == "false") ? false : true; }

		friend std::ostream& operator<<(std::ostream& out, const SMAnsiString& s)
		{
			out << s.c_str();
			return out;
		}

		friend SMAnsiString operator+(const char* lvalue, const SMAnsiString& rvalue)
		{
			SMAnsiString ret(lvalue);
			ret += rvalue;
			return std::move(ret);
		}

#if 0 
		void* operator new(size_t size)
		{
			return malloc(size);
		}
#endif
};

#endif
