#ifndef TGBOT_UTILS_HEADER
#define TGBOT_UTILS_HEADER

#include "tg_bot.h"

typedef std::vector<SMAnsiString>          StringList;

using StringListPtr                = std::unique_ptr<StringList>;
using HTTPResponsePtr              = std::unique_ptr<HTTP_Response>;

#define DELETE_SINGLE_OBJECT(_mptr) if(_mptr != nullptr) { delete _mptr; _mptr = nullptr; }
#define DELETE_ARRAY_OBJECT(_mptr) if(_mptr != nullptr) { delete[] _mptr; _mptr = nullptr; }

#define SizeOfArray(a) (sizeof(a)/sizeof(a[0]))

enum
{
	TGB_TEXTCOLOR_DEFAULT = 0,
	TGB_TEXTCOLOR_YELLOW,
	TGB_TEXTCOLOR_GREEN,
	TGB_TEXTCOLOR_RED,
	TGB_TEXTCOLOR_LIGHTBLUE,
	TGB_TEXTCOLOR_PURPURE,
	TGB_TEXTCOLOR_WHITE,
	TGB_TEXTCOLOR_BLUE
};

#ifdef _WIN32
void usleep(unsigned int usec);
#endif
SMAnsiString ExtractFilePath(const SMAnsiString &FileName);
bool LoadLastBotState(const SMAnsiString &FileName);
bool SaveLastBotState(const SMAnsiString &FileName);
void WriteMessage(const SMAnsiString &Message, int Color=TGB_TEXTCOLOR_DEFAULT);
void WriteFormatMessage(const SMAnsiString& Format, int Color, ...);
void AppendStringToLog(const SMAnsiString &filename, const SMAnsiString& Message);
StringList* LoadTextFromFile(const SMAnsiString &FileName);
bool LoadTextFromFile_v2(const SMAnsiString &FileName, StringList*Output);
SMAnsiString TimeDifferenceString(time_t Time1, time_t Time2);
void Hexlify(char* hex, const unsigned char* bin, int len);
SMAnsiString ParamsToJSON(const StringList& keys, const StringList &values);
void ParamsFromJSON(const SMAnsiString &json, const StringList &keys, StringList &values);
SMAnsiString MakeKeyListStr(const SMKeyList& list);
int DayOfWeekCovert(int Src);
int WhatDay(int D, int M, int Y);
int GetDays(int month, int year);
bool GetDateFromString(const SMAnsiString& str, int& day, int& month, int& year);
SMAnsiString MakePermissionString(uint64_t permission, const StringList* str = nullptr);
time_t GetBeginDay(time_t _tm);
time_t GetBeginDayGM(time_t _gtm);

// MEM.H
//void* operator new(std::size_t size);

#endif