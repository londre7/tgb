#include "tg_bot.h"

bool LoadConfFile(BaseConf* ConfStruct, const SMAnsiString& FileName)
{
	int							assign_pos;
	SMAnsiString   					key;
	SMAnsiString   					value;
	size_t							file_count;

	// считываем из файла
	std::unique_ptr< StringList> file(LoadTextFromFile(FileName));
	if (file == NULL) return false;
	if (!file->size())
	{
		return false;
	}

	file_count = file->size();

	for (size_t i = 0; i < file_count; i++)
	{
		if (file->operator[](i).length() == 2)
		{
			if ((file->operator[](i)[0] != '/') && (file->operator[](i)[1] != '/'))
			{
				file->clear();
				return false;
			}
		}
		else if (file->operator[](i).length() > 2)
		{
			if ((file->operator[](i)[0] != L'/') && (file->operator[](i)[1] != L'/'))
			{
				assign_pos = file->operator[](i).Pos('=');
				if (assign_pos == -1)
				{
					file->clear();
					return false;
				}

				key = file->operator[](i);
				key = key.Delete(assign_pos, key.length());
				value = file->operator[](i);
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
		else if (file->operator[](i).length() == 1)
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