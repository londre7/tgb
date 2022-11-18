#include "tg_bot.h"

bool LoadConfFile(BaseConf* ConfStruct, const SMAnsiString& FileName)
{
	// считываем из файла
	std::unique_ptr<StringList> file(LoadTextFromFile(FileName));
	if (file == NULL) return false;
	const size_t file_count = file->size();
	if (!file_count) { return false; }

	int assign_pos;
	SMAnsiString key;
	SMAnsiString value;
	for (size_t i = 0; i < file_count; i++)
	{
		SMAnsiString &fstr = file->at(i);
		if (fstr.length() == 2)
		{
			if ((fstr[0] != '/') && (fstr[1] != '/'))
				return false;
		}
		else if (fstr.length() > 2)
		{
			if ((fstr[0] != L'/') && (fstr[1] != L'/'))
			{
				assign_pos = fstr.Pos('=');
				if (assign_pos == -1)
					return false;

				key = fstr.Delete(assign_pos, fstr.length());
				fstr.DeleteMyself(0, assign_pos + 1);

				if (!key.length())
					return false;
				if (!fstr.length())
					return false;

				ConfStruct->SetParam(key, fstr);
			}
		}
		else if (fstr.length() == 1)
			return false;
	}
	return true;
}

BotConfStruct BotConf;			// конфиг bot.conf
BotConfStruct* GetBotConf() { return &BotConf; }