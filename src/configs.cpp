#include "tg_bot.h"

bool LoadConfFile(BaseConf* ConfStruct, const SMAnsiString& FileName)
{
	// считываем из файла
	std::unique_ptr<StringList> file(LoadTextFromFile(FileName));
	if (file == NULL) return false;
	if (!file->size())
	{
		return false;
	}

	SMAnsiString key;
	SMAnsiString value;
	int assign_pos;
	const size_t file_count = file->size();
	for (size_t i = 0; i < file_count; i++)
	{
		if (file->at(i).length() == 2)
		{
			if ((file->at(i)[0] != '/') && (file->at(i)[1] != '/'))
			{
				file->clear();
				return false;
			}
		}
		else if (file->at(i).length() > 2)
		{
			if ((file->at(i)[0] != L'/') && (file->at(i)[1] != L'/'))
			{
				assign_pos = file->at(i).Pos('=');
				if (assign_pos == -1)
				{
					file->clear();
					return false;
				}

				key = file->at(i);
				key = key.Delete(assign_pos, key.length());
				value = file->at(i);
				value = value.Delete(0, assign_pos + 1);

				if (!key.length())
				{
					file->clear();
					return false;
				}
				if (!value.length())
				{
					file->clear();
					return false;
				}

				ConfStruct->SetParam(key, value);
			}
		}
		else if (file->at(i).length() == 1)
		{
			file->clear();
			return false;
		}
	}

	file->clear();
	return true;
}

BotConfStruct BotConf;			// конфиг bot.conf
BotConfStruct* GetBotConf() { return &BotConf; }