#ifndef TGBOT_SOCKET_HEADER
#define TGBOT_SOCKET_HEADER

#include "tg_bot.h"

#ifdef _WIN32
#define TGB_CLOSESOCKET(socket) closesocket(socket)
#define REINTERPRET_CAST_CHARPTR(buf) reinterpret_cast<char*>(buf)
#else
#define TGB_CLOSESOCKET(socket) close(socket)
#define REINTERPRET_CAST_CHARPTR(buf) buf
#endif

int SockConnect(int &SockFD, SMAnsiString IP, int Port);
int SockConnectAsync(int &SockFD, SMAnsiString IP, int Port);
SSL* SockConnectWithSSL(int &SockFD, SMAnsiString IP, int Port, int &Err);
bool Is_IPv4(SMAnsiString Host);
SMAnsiString GetIPFromHost(SMAnsiString Host);

#endif