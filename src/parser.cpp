#include "tg_bot.h"

static SMAnsiString* ParseFormatString(const SMAnsiString &Format, int &PCount)
{
	SMAnsiString Param = "param";
	SMAnsiString Raw = "raw";
	char         *d_buf;
	SMAnsiString *OutParams;
	
	if(Format == "") return NULL;
	if(Format[Format.length()-1] == ';') return NULL;
	
	// оперделяем количество параметров
	PCount = 1;
	for(int i=0; i<Format.length(); i++)
	{
		if(Format[i] == ';') PCount++; 
	}
	
	// выделяем память
	OutParams = new SMAnsiString[PCount];
	
	for(int i=0, b_pos=0, e_pos=0, a_pos=0; i<Format.length(); i++)
	{
		if(Format[i] == ';')
		{
			e_pos = i-1;
			d_buf = new char[e_pos-b_pos+2];
			memcpy(d_buf, &Format.c_str()[b_pos], e_pos-b_pos+1);
			d_buf[e_pos-b_pos+1] = '\0';
			OutParams[a_pos] = d_buf;
			a_pos++;
			b_pos = i+1;
			try { delete[] d_buf; } catch(...) { ; }
		}
		
		if(i == Format.length()-1)
		{
			e_pos = i;
			d_buf = new char[e_pos-b_pos+2];
			memcpy(d_buf, &Format.c_str()[b_pos], e_pos-b_pos+1);
			d_buf[e_pos-b_pos+1] = '\0';
			OutParams[a_pos] = d_buf;
			try { delete[] d_buf; } catch(...) { ; }
		}
	}
	
	return OutParams;
}

StringList* ParseFormatString(const SMAnsiString& Format)
{
	int pcount;
	SMAnsiString* p = ParseFormatString(Format, pcount);
	if (!p) return nullptr;
	StringList* list = new StringList;
	for (int i = 0; i < pcount; i++)
	{
		list->push_back(p[i]);
	}
	DELETE_ARRAY_OBJECT(p);
	return list;
}
