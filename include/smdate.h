#ifndef SMDATE_HEADER
#define SMDATE_HEADER

#include <time.h>
#include "smstring.h"

#define DATESTRING "%02d%c%02d%c%04d"
#define TIMESTRING "%02d%c%02d%c%02d"

class SMDateTime	// дата и время
{
	protected:
		time_t _LowLevelTime;

	public:
		// конструткоры
		SMDateTime(): _LowLevelTime(time(NULL)) {}
		SMDateTime(const SMDateTime& dt) { _LowLevelTime = dt._LowLevelTime; }
		SMDateTime(time_t Tm) { _LowLevelTime = Tm; }
		SMDateTime(int year, int month, int day, int hour, int min, int sec)
		{
			tm time_structure;
			time_structure.tm_year = year-1900;
			time_structure.tm_mon = month-1;
			time_structure.tm_mday = day;
			time_structure.tm_hour = hour;
			time_structure.tm_min = min;
			time_structure.tm_sec = sec;
			_LowLevelTime = mktime(&time_structure);
		}
		
		// методы
		void SetTime(time_t tm) { _LowLevelTime = tm; }
		time_t GetTime() { return _LowLevelTime; }
		//- получить текущее время
		void Now() { _LowLevelTime = time(NULL); }
		//- текущий месяц
		int GetMonth() const { return localtime(&_LowLevelTime)->tm_mon+1; }
		int GetYear() const { return localtime(&_LowLevelTime)->tm_year + 1900; }
		int GetDay() const { return localtime(&_LowLevelTime)->tm_mday; }
		int GetHour() const { return localtime(&_LowLevelTime)->tm_hour; }
		//- время time_t начала дня (локально)
		time_t DayBegin() const
		{
			tm time_structure;
			time_structure.tm_year = this->GetYear() - 1900;
			time_structure.tm_mon = this->GetMonth() - 1;
			time_structure.tm_mday = this->GetDay();
			time_structure.tm_hour = 0;
			time_structure.tm_min = 0;
			time_structure.tm_sec = 0;
			return mktime(&time_structure);
		}
		//- время в формате строки ДД.ММ.ГГГГ ЧЧ:ММ::СС
		SMAnsiString DateTimeString(char date_delim='.', char time_delim=':', char sp_delim=' ') const
		{
			tm *curtime = localtime(&_LowLevelTime);
			return std::move
			(
				SMAnsiString::smprintf
				(
					DATESTRING "%c" TIMESTRING,
					curtime->tm_mday,
					date_delim,
					curtime->tm_mon + 1,
					date_delim,
					curtime->tm_year + 1900,
					sp_delim,
					curtime->tm_hour,
					time_delim,
					curtime->tm_min,
					time_delim,
					curtime->tm_sec
				)
			);
		}
		//- время в формате строки ДД_ММ_ГГГГ_ЧЧ_ММ_СС
		SMAnsiString DateTimeSpString() const
		{
			return std::move(DateTimeString('_', '_', '_'));
		}
		//- GMT 0 время в формате строки ДД.ММ.ГГГГ ЧЧ:ММ::СС
		SMAnsiString GMDateTimeString(char date_delim = '.', char time_delim = ':', char sp_delim = ' ') const
		{
			tm* curtime = gmtime(&_LowLevelTime);
			return std::move
			(
				SMAnsiString::smprintf
				(
					DATESTRING "%c" TIMESTRING,
					curtime->tm_mday,
					date_delim,
					curtime->tm_mon + 1,
					date_delim,
					curtime->tm_year + 1900,
					sp_delim,
					curtime->tm_hour,
					time_delim,
					curtime->tm_min,
					time_delim,
					curtime->tm_sec
				)
			);
		}
		//- локальное время в формате ЧЧ:ММ::СС
		SMAnsiString TimeString(char delim=':') const
		{
			tm* curtime = localtime(&_LowLevelTime);
			return std::move
			(
				SMAnsiString::smprintf
				(
					TIMESTRING,
					curtime->tm_hour,
					delim,
					curtime->tm_min,
					delim,
					curtime->tm_sec
				)
			);
		}
		//- GMT0 время в формате ЧЧ:ММ::СС
		SMAnsiString GMTimeString(char delim=':') const
		{
			tm* curtime = gmtime(&_LowLevelTime);
			return std::move
			(
				SMAnsiString::smprintf
				(
					TIMESTRING,
					curtime->tm_hour,
					delim,
					curtime->tm_min,
					delim,
					curtime->tm_sec
				)
			);
		}
		SMAnsiString DateString(char delim='.') const
		{
			tm* curtime = localtime(&_LowLevelTime);
			return std::move
			(
				SMAnsiString::smprintf
				(
					DATESTRING,
					curtime->tm_mday,
					delim,
					curtime->tm_mon + 1,
					delim,
					curtime->tm_year + 1900
				)
			);
		}
};

#endif
