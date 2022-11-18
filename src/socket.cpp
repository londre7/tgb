#include "socket.h"

int SockConnect(int &SockFD, const SMAnsiString &IP, int Port)
{
	struct sockaddr_in	servaddr;
	
	SockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(Port);
	return connect(SockFD, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

int SockConnectAsync(int &SockFD, const SMAnsiString &IP, int Port)
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

SSL* SockConnectWithSSL(int &SockFD, const SMAnsiString &IP, int Port, int &Err)
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
	
	SSL_set_fd(ssl, SockFD);
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

	const int hostLen = Host.length();
	if ((hostLen < 7) || (hostLen > 15)) return false;

	char bp[16];
	char *bppos = bp;
	SMAnsiString b[4];
	size_t cb = 0ull;
	const int hlen = Host.length();
	for (int i = 0; i < hlen; i++)
	{
		char &l = Host[i];
		if (i == (hlen - 1))
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
	if (cb != 4) return false;
	return true;
}

SMAnsiString GetIPFromHost(const SMAnsiString &Host)
{
	SMAnsiString ret;
	if(!Is_IPv4(Host))
	{
		// получаем IP из DNS
		struct hostent *hostinfo = gethostbyname(Host.c_str());
		if(hostinfo == NULL) return "";  // dns not resolved
		
		for(int i=0; i<4; i++)
		{
			if (i) ret += '.';
			ret += SMAnsiString((int)uint8_t(hostinfo->h_addr_list[0][i]));
		}
	}
	else ret = Host;
	return std::move(ret);
}
