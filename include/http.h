#ifndef TGBOT_HTTP_HEADER
#define TGBOT_HTTP_HEADER

#include "tg_bot.h"
#include "custombuffer.h"

struct HTTP_Response
{
	SMAnsiString HTTPVer;
	SMAnsiString ErrCode;
	SMAnsiString ContentType;
	SMAnsiString DateStr;
	size_t       ContentLength = 0ull;
	SMAnsiString Content;
};

extern HTTP_Response* HTTP_Get(const SMAnsiString &Host, int Port, bool UseSSL, const SMAnsiString &HeaderHost, const SMAnsiString &Doc, int &Err);
extern HTTP_Response* HTTP_Post(const SMAnsiString &Host, int Port, bool UseSSL, const SMAnsiString &HeaderHost, const SMAnsiString &Doc, const char *ContentType, void *Content, size_t ContentLength, int &Err);

#endif