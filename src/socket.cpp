#include "socket.h"

int SockConnect(int &SockFD, SMAnsiString IP, int Port)
{
	struct sockaddr_in	servaddr;
	
	SockFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(Port);
	return connect(SockFD, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

int SockConnectAsync(int &SockFD, SMAnsiString IP, int Port)
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

SSL* SockConnectWithSSL(int &SockFD, SMAnsiString IP, int Port, int &Err)
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

bool Is_IPv4(SMAnsiString Host)
{
	int		pt_count;
	char	num[16];
	int		val;
	
	if((Host.length() < 7) || (Host.length() > 15)) return false;
	
	pt_count = 0;
	for(int i=0; i<Host.length(); i++)
	{
		if((((int)Host[i] >= 48) && ((int)Host[i] <= 57)) || (Host[i] == '.')) 
		{
			if(Host[i] == '.') 
			{
				if(i != 0)
				{
					if(Host[i-1] == '.') return false;
					else pt_count++;
				}
				else pt_count++;
			}
		}
		else return false;
	}
	
	if(pt_count != 3) return false;
	
	// смотрим, чтобы числа были от 0 до 255
	for(int cnt=0, i=0; cnt<4; cnt++)
	{
		memset(num, 0, 16);
		for(int j=0; true; j++)
		{
			if(Host[i] != '.')
			{
				num[j] = Host[i];
				i++;
				if(i >= Host.length())
				{
					//num[j] = '\0';
					val = atoi(num);
					if(val > 255) return false;
					else break;
				}
			}
			else
			{
				//num[j] = '\0';
				val = atoi(num);
				if(val > 255) return false;
				else 
				{
					i++;
					break;
				}
			}
		}
	}
	
	return true;
}

SMAnsiString GetIPFromHost(SMAnsiString Host)
{
	struct hostent	*hostinfo;
	SMAnsiString	ret;
	
	if(!Is_IPv4(Host))
	{
		// пролукчаем IP из DNS
		hostinfo = gethostbyname(Host.c_str());
		if(hostinfo == NULL) return "";  // dns not resolved
		
		for(int i=0; i<4; i++)
		{
			ret = ret + SMAnsiString((int)uint8_t(hostinfo->h_addr_list[0][i]));
			if(i != 3) ret = ret + ".";
		}
	}
	else ret = Host;
	
	return ret;
}
