#include "socket.h"
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

int SockConnect(SOCKTYPE &SockFD, const SMAnsiString &IP, int Port)
{
	struct sockaddr_in	servaddr;
	
	SockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(Port);
	return connect(SockFD, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

int SockConnectAsync(SOCKTYPE &SockFD, const SMAnsiString &IP, int Port)
{
	struct sockaddr_in	servaddr;
	
	SockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifndef _WIN32
	fcntl(SockFD, F_SETFL, O_NONBLOCK);
#else
	bool b = true;
	ioctlsocket(SockFD, FIONBIO, (unsigned long*)& b);
#endif
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(Port);
	return connect(SockFD, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

SSL* SockConnectWithSSL(SOCKTYPE &SockFD, const SMAnsiString &IP, int Port, int &Err)
{
	SSL *ssl;
	struct sockaddr_in	servaddr;
	int s_err;
	
	// соединяемся
	SockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(Port);
	s_err = connect(SockFD, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(s_err == -1)
	{
		Err = -777;
		return NULL;
	}
	
	// прикручиваем SSL
	const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    ssl = SSL_new (ctx);
	SSL_CTX_free(ctx);
	
	SSL_set_fd(ssl, (int)SockFD);
	s_err = SSL_connect(ssl);
	
	if(s_err <= 0)
	{
		Err = s_err;
		return NULL;
	}
	
	return ssl;
}

bool Is_IPv4(const SMAnsiString &Host)
{
	#define DIGIT \
		*bppos = l; \
		++bppos;
	#define DELIMITER \
		*bppos = '\0'; \
		b[cb] = bp; \
		if ((int(b[cb]) < 0) || (int(b[cb]) > 255)) \
			return false; \
		bppos = bp; \
		++cb; 

	const size_t hostLen = Host.length();
	if ((hostLen < 7ull) || (hostLen > 15ull)) return false;

	char bp[16];
	char *bppos = bp;
	SMAnsiString b[4];
	size_t cb = 0ull;
	const size_t hlen = Host.length();
	for (size_t i = 0; i < hlen; i++)
	{
		char &l = Host[i];
		if (i == (hlen - 1ull))
		{
			DIGIT;
			DELIMITER;
		}
		else if (isdigit((int)l))
		{
			// цифра
			DIGIT;
		}
		else if (l == '.')
		{
			// разделитель
			DELIMITER;
		}
		else
			return false;
	}
	if (cb != 4ull) return false;
	return true;
}

// TODO: сделать поддержку IPv6
SMAnsiString GetIPFromHost(const SMAnsiString &Host)
{
	SMAnsiString ret;
	static char addrbuf[INET_ADDRSTRLEN];
	if (!Is_IPv4(Host))
	{
		struct addrinfo* res = NULL;
		if (!getaddrinfo(Host.c_str(), nullptr, nullptr, &res) && res) // succes return zero
		{
			void* ptr = nullptr;
			auto *addrstruct = res;
			while (addrstruct)
			{
				ptr = nullptr;
				switch (addrstruct->ai_family)
				{
					case AF_INET:
						ptr = &((struct sockaddr_in*) addrstruct->ai_addr)->sin_addr;
						break;
					//case AF_INET6:
					//	ptr = &((struct sockaddr_in6*) res->ai_addr)->sin6_addr;
					//	break;
				}
				if (ptr)
				{
					inet_ntop(AF_INET, ptr, addrbuf, INET_ADDRSTRLEN);
					if (strcmp(addrbuf, "0.0.0.0"))
					{
						ret = addrbuf;
						break;
					}
				}	
				addrstruct = addrstruct->ai_next;
			}
			freeaddrinfo(res);
		}
		else
			CLEAR_STR(ret);
	}
	else
		ret = Host;
	return std::move(ret);
}
