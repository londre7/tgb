#include "tg_bot.h"

StringList* ParseFormatString(const SMAnsiString& Format)
{
	StringList* list = new StringList;
	if (Format.IsEmpty()) return list;

	const int length = Format.length();
	for (int i=0, b_pos=0; i < length; i++)
	{
		if (Format[i] == ';')
		{
			list->push_back(SMAnsiString(&Format[b_pos], i-b_pos, 0));
			b_pos = i + 1;
		}
		if ((i == (Format.length()-1)) && (Format[i]!=';'))
			list->push_back(SMAnsiString(&Format[b_pos], i-b_pos+1, 0));
	}
	return list;
}