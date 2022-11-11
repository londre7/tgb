#ifndef SMDATE_HEADER
#define SMDATE_HEADER

#include <time.h>
#include "smstring.h"

class SMDateTime	// дата и время
{
	public:
		time_t	LowLevelTime;
	
		// конструткоры
		SMDateTime(): LowLevelTime(time(NULL)) {}
		SMDateTime(const SMDateTime& dt) { this->LowLevelTime = dt.LowLevelTime; }
		SMDateTime(time_t Tm) { LowLevelTime = Tm; }
		SMDateTime(int year, int month, int day, int hour, int min, int sec)
		{
			tm time_structure;
			time_structure.tm_year = year-1900;
			time_structure.tm_mon = month-1;
			time_structure.tm_mday = day;
			time_structure.tm_hour = hour;
			time_structure.tm_min = min;
			time_structure.tm_sec = sec;
			LowLevelTime = mktime(&time_structure);
		}
		
		// методы
		//- получить текущее время
		void Now() { LowLevelTime = time(NULL); }
		//- текущий месяц
		int GetMonth() const
		{
			tm* CurTime;
			CurTime = localtime(&LowLevelTime);
			return CurTime->tm_mon+1;
		}
		int GetYear() const
		{
			return localtime(&LowLevelTime)->tm_year + 1900;
		}
		int GetDay() const
		{
			return localtime(&LowLevelTime)->tm_mday;
		}
		int GetHour() const
		{
			return localtime(&LowLevelTime)->tm_hour;
		}
		//- время начала дня (локально)
		time_t DayBegin()
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
		SMAnsiString DateTimeString()
		{
			tm				*CurTime;
			SMAnsiString	Returned;
			
			CurTime = localtime(&LowLevelTime);
			
			if(CurTime->tm_mday < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mday);
			else Returned = Returned + SMAnsiString(CurTime->tm_mday);
			Returned = Returned + ".";
			if((CurTime->tm_mon+1) < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mon+1);
			else Returned = Returned + SMAnsiString(CurTime->tm_mon+1);
			Returned = Returned + ".";
			Returned = Returned + SMAnsiString(1900 + CurTime->tm_year);
			Returned = Returned + " ";
			if(CurTime->tm_hour < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_hour);
			else Returned = Returned + SMAnsiString(CurTime->tm_hour);
			Returned = Returned + ":";
			if(CurTime->tm_min < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_min);
			else Returned = Returned + SMAnsiString(CurTime->tm_min);
			Returned = Returned + ":";
			if(CurTime->tm_sec < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_sec);
			else Returned = Returned + SMAnsiString(CurTime->tm_sec);
			
			return Returned;
		}
		//- время в формате строки ДД_ММ_ГГГГ_ЧЧ_ММ_СС
		SMAnsiString DateTimeSpString()
		{
			tm				*CurTime;
			SMAnsiString	Returned;
			
			CurTime = localtime(&LowLevelTime);
			
			if(CurTime->tm_mday < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mday);
			else Returned = Returned + SMAnsiString(CurTime->tm_mday);
			Returned = Returned + "_";
			if((CurTime->tm_mon+1) < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mon+1);
			else Returned = Returned + SMAnsiString(CurTime->tm_mon+1);
			Returned = Returned + "_";
			Returned = Returned + SMAnsiString(1900 + CurTime->tm_year);
			Returned = Returned + "_";
			if(CurTime->tm_hour < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_hour);
			else Returned = Returned + SMAnsiString(CurTime->tm_hour);
			Returned = Returned + "_";
			if(CurTime->tm_min < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_min);
			else Returned = Returned + SMAnsiString(CurTime->tm_min);
			Returned = Returned + "_";
			if(CurTime->tm_sec < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_sec);
			else Returned = Returned + SMAnsiString(CurTime->tm_sec);
			
			return Returned;
		}
		//- GMT 0 время в формате строки ДД.ММ.ГГГГ ЧЧ:ММ::СС
		SMAnsiString GMDateTimeString()
		{
			tm				*CurTime;
			SMAnsiString	Returned;
			
			CurTime = gmtime(&LowLevelTime);
			
			if(CurTime->tm_mday < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mday);
			else Returned = Returned + SMAnsiString(CurTime->tm_mday);
			Returned = Returned + ".";
			if((CurTime->tm_mon+1) < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_mon+1);
			else Returned = Returned + SMAnsiString(CurTime->tm_mon+1);
			Returned = Returned + ".";
			Returned = Returned + SMAnsiString(1900 + CurTime->tm_year);
			Returned = Returned + " ";
			if(CurTime->tm_hour < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_hour);
			else Returned = Returned + SMAnsiString(CurTime->tm_hour);
			Returned = Returned + ":";
			if(CurTime->tm_min < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_min);
			else Returned = Returned + SMAnsiString(CurTime->tm_min);
			Returned = Returned + ":";
			if(CurTime->tm_sec < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_sec);
			else Returned = Returned + SMAnsiString(CurTime->tm_sec);
			
			return Returned;
		}
		//- локальное время в формате ЧЧ:ММ::СС
		SMAnsiString TimeString()
		{
			tm				*CurTime;
			SMAnsiString	Returned;
			
			CurTime = localtime(&LowLevelTime);
			
			Returned = "";
			Returned = Returned + SMAnsiString(CurTime->tm_hour);
			Returned = Returned + ":";
			if(CurTime->tm_min < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_min);
			else Returned = Returned + SMAnsiString(CurTime->tm_min);
			Returned = Returned + ":";
			if(CurTime->tm_sec < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_sec);
			else Returned = Returned + SMAnsiString(CurTime->tm_sec);
			
			return Returned;
		}
		//- GMT0 время в формате ЧЧ:ММ::СС
		SMAnsiString GMTimeString()
		{
			tm				*CurTime;
			SMAnsiString	Returned;
			
			CurTime = gmtime(&LowLevelTime);
			
			Returned = "";
			Returned = Returned + SMAnsiString(CurTime->tm_hour);
			Returned = Returned + ":";
			if(CurTime->tm_min < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_min);
			else Returned = Returned + SMAnsiString(CurTime->tm_min);
			Returned = Returned + ":";
			if(CurTime->tm_sec < 10) Returned = Returned + "0" + SMAnsiString(CurTime->tm_sec);
			else Returned = Returned + SMAnsiString(CurTime->tm_sec);
			
			return Returned;
		}
		SMAnsiString DateString()
		{
			tm* ct = localtime(&LowLevelTime);
			return SMAnsiString::smprintf("%02d.%02d.%d", ct->tm_mday, ct->tm_mon+1, ct->tm_year + 1900);
		}
};

#endif
