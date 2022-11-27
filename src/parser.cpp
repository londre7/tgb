#include "tg_bot.h"

StringList* ParseFormatString(const SMAnsiString& Format)
{
	StringList* list = new StringList;
	if (Format.IsEmpty()) return list;

	const size_t length = Format.length();
	for (size_t i=0, b_pos=0; i < length; i++)
	{
		if (Format[i] == ';')
		{
			list->push_back(SMAnsiString(&Format[b_pos], i-b_pos, 0));
			b_pos = i + 1;
		}
		if ((i == (Format.length()-1ull)) && (Format[i]!=';'))
			list->push_back(SMAnsiString(&Format[b_pos], i-b_pos+1, 0));
	}
	return list;
}