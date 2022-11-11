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
	unsigned int ContentLength;
    SMAnsiString Content;

	HTTP_Response() : Content(65536ull) {}
};

extern HTTP_Response* ParseHTTPResponse(char *SrcData, unsigned int SrcLength);
extern HTTP_Response* HTTP_Get(SMAnsiString Host, int Port, bool UseSSL, SMAnsiString HeaderHost, SMAnsiString Doc, int &Err);
extern HTTP_Response* HTTP_Post(SMAnsiString Host, int Port, bool UseSSL, SMAnsiString HeaderHost, SMAnsiString Doc, const char *ContentType, void *Content, size_t ContentLength, int &Err);
extern HTTP_Response* HTTP_Connect(SMAnsiString Host, int Port, bool UseSSL, SMAnsiString HeaderHost, SMAnsiString Doc, int &Err);
extern HTTP_Response* HTTP_Post_With_Content_v2(SMAnsiString Host, int Port, bool Tunnel, SMAnsiString HeaderHost, SMAnsiString Doc, const char *ContentType, void *Content, int ContentLength, int &Err);
extern HTTP_Response* HTTP_Post_Without_Response(SMAnsiString Host, int Port, bool Tunnel, SMAnsiString HeaderHost, SMAnsiString Doc, const char *ContentType, void *Content, int ContentLength, int &Err);
extern HTTP_Response* HTTP_Post_With_Content_v3(SMAnsiString Host, int Port, bool Tunnel, bool Async, int Timeout, SMAnsiString HeaderHost, SMAnsiString Doc, const char *ContentType, void *Content, int ContentLength, int &Err);

#endif