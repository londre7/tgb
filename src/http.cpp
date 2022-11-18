#include "tg_bot.h"

static HTTP_Response* RecvResponse(SSL *sock_ssl, int &Err)
{
	auto SET_HEADER_PARAM = [](SMAnsiString Key, const SMAnsiString &Value, HTTP_Response *HeaderData)
	{
		Key.ToLowerCase();
		if (Key == "content-type") HeaderData->ContentType = Value;
		else if (Key == "date") HeaderData->DateStr = Value;
		else if (Key == "content-length") HeaderData->ContentLength = Value;
		else return;
	};

	char buffer[8192];
	int socket_err;

	// получаем первый пакет, анализируем
	memset(buffer, 0, 8192);
	for (int i = 0; 1; i++)
	{
		socket_err = SSL_read(sock_ssl, buffer, 8192);

		if (socket_err < 0) { Err = 1003; return nullptr; } // error receiving
		else if (socket_err == 0)
		{
			if (i > 65000) break;
			continue;
		}
		else break;
	}

	// проверяем, являются ли полученные данные HTTP
	char http_signature[5];
	memset(http_signature, 0, 5);
	memcpy(http_signature, buffer, 4);
	if(*reinterpret_cast<const uint32_t*>(http_signature) != *reinterpret_cast<const uint32_t*>("HTTP"))
	{
		Err = 1004;	// empty response
		return nullptr;
	}

	HTTP_Response *resp = new HTTP_Response;
	resp->ContentLength = 0;
	resp->ContentType = "";

	// разбираем версию HTTP и код ошибки
	char *ptrb = buffer;
	size_t stop_index;
	size_t recv_buf_len = strlen(buffer);
	for (size_t i = 0, round = 0, breakflag = 0; i < recv_buf_len; i++)
	{
		switch (round)
		{
			case 0:
			{
				if(buffer[i] == ' ')
				{
					buffer[i] = '\0';
					resp->HTTPVer = ptrb;
					round++;
					ptrb = &buffer[i + 1];
				}
				break;
			}
			case 1:
			{
				if (buffer[i] == '\r')
					buffer[i] = '\0';
				else if (buffer[i] == '\n')
				{
					buffer[i] = '\0';
					resp->ErrCode = ptrb;
					breakflag = 1;
					stop_index = i + 1;
				}
				break;
			}
		}

		if (breakflag == 1) break;
	}

	// разбираем всё остальное
	char *ptrk = &buffer[stop_index];
	char *ptrv = nullptr;
	for (size_t i = stop_index, state = 0, beg, breakflag = 0; i < recv_buf_len; i++)
	{
		switch (state)
		{
			case 0: // парсим параметр
			{
				if (buffer[i] == '\n')
				{
					stop_index = i + 1;
					breakflag = 1;
					break;
				}
				else if (buffer[i] == ':')
				{
					buffer[i] = '\0';
					ptrv = &buffer[i+2];
					beg = i + 1;
					state = 1;
				}
				break;
			}
			case 1: // парсим значение
			{
				if (buffer[i] == '\r')
				{
					buffer[i] = '\0';
					break;
				}
				else if (buffer[i] == '\n')
				{
					buffer[i] = '\0';
					SET_HEADER_PARAM(ptrk, ptrv, resp);
					ptrk = &buffer[i + 1];
					state = 0;
				}
				break;
			}
		}
		if (breakflag == 1) break;
	}

	if (resp->ContentLength > 0)
	{
		//resp->Content = "";
		if (strlen(buffer) - stop_index != 0)
		{
			// контент тут, копируем
			resp->Content = &buffer[stop_index];
		}

		while (true)
		{
			if (resp->Content.length() < resp->ContentLength)
			{
				memset(buffer, 0, 8192);
				socket_err = SSL_read(sock_ssl, buffer, 8192);

				if (socket_err <= 0) { delete resp; Err = 1005; return NULL; }

				resp->Content += buffer;
			}
			else break;
		}
	}
	else
	{
		//recv_buf_len = strlen(buffer);
		if (recv_buf_len - stop_index != 0)
		{
			// сначала длина контента, потом сам контент
			ptrb = &buffer[stop_index];
			for (size_t i = stop_index; i < recv_buf_len; i++)
			{
				if (buffer[i] == '\r')
				{
					buffer[i] = '\0';
					continue;
				}
				else if (buffer[i] == '\n')
				{
					buffer[i] = '\0';
					stop_index = i + 1;
					break;
				}
			}
			resp->ContentLength = atoi(ptrb);
			resp->Content = &buffer[stop_index];
			while (true)
			{
				if (resp->Content.length() < resp->ContentLength)
				{
					memset(buffer, 0, 8192);
					socket_err = SSL_read(sock_ssl, buffer, 8192);

					if (socket_err <= 0) { delete resp; Err = 1005; return nullptr; }

					resp->Content += buffer;
				}
				else break;
			}
		}
		// убираем /r/n в конце
		if (resp->Content[resp->Content.length() - 2] == '\r')
			resp->Content[resp->Content.length() - 2] = '\0';
	}
	
	return resp;
}

HTTP_Response* HTTP_Get(const SMAnsiString &Host, int Port, bool UseSSL, const SMAnsiString &HeaderHost, const SMAnsiString &Doc, int& Err)
{
	// получаем IP-адрес из имени хоста
	SMAnsiString ip_by_host = GetIPFromHost(Host);
	if (ip_by_host.IsEmpty()) { Err = 1000; return NULL; }  // dns not resolved

	// соединяемся с сервером
	int sock;
	int	socket_err;
	SSL* sock_ssl = nullptr;
	if (UseSSL)
	{
		sock_ssl = SockConnectWithSSL(sock, ip_by_host, Port, socket_err);
		if (sock_ssl == NULL)
		{
			Err = 1001;	// couldn't connect
			return NULL;
		}
	}
	else
	{
		socket_err = SockConnect(sock, ip_by_host, Port);
		if (socket_err == -1) { Err = 1001; return NULL; } // couldn't connect
	}

	// делаем запрос
	SMAnsiString get_doc = ((UseSSL) ? "https://" : "http://") + HeaderHost + Doc;
	SMAnsiString req = SMAnsiString::smprintf
	(
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: keep-alive\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"Sec-Fetch-User: ?1\r\n"
		"Accept: */*\r\n"
		"Sec-Fetch-Site: none\r\n"
		"Sec-Fetch-Mode: navigate\r\n"
		"Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
		"\r\n",
		C_STR(get_doc),
		C_STR(HeaderHost)
	);
	if (UseSSL)
	{
		socket_err = SSL_write(sock_ssl, req.c_str(), req.length());
		if (socket_err <= 0) { Err = 1002; return NULL; } // error sending
	}
	else
	{
		socket_err = send(sock, req.c_str(), req.length(), 0);
		if (socket_err == -1) { Err = 1002; return NULL; } // error sending
	}

	// получаем ответ
	HTTP_Response *resp = (UseSSL)?RecvResponse(sock_ssl, Err):nullptr; // TODO: сделать для обычного сокета
	
	// закрываем соединение
	if (UseSSL)
	{
		SSL_shutdown(sock_ssl);
		SSL_free(sock_ssl);
	}
	TGB_CLOSESOCKET(sock);
	
	return resp;
}

HTTP_Response* HTTP_Post(const SMAnsiString &Host, int Port, bool UseSSL, const SMAnsiString &HeaderHost, const SMAnsiString &Doc, const char *ContentType, void *Content, size_t ContentLength, int &Err)
{
	// получаем IP-адрес из имени хоста
	SMAnsiString ip_by_host = GetIPFromHost(Host);
	if(ip_by_host == "") { Err = 1000; return NULL; }  // dns not resolved
	
	// соединяемся с сервером
	int	sock;
	int	socket_err;
	SSL* sock_ssl = nullptr;
	if(UseSSL) 
	{
		sock_ssl = SockConnectWithSSL(sock, ip_by_host, Port, socket_err);
		if(sock_ssl == NULL)
		{
			Err = 1001;	// couldn't connect
			return NULL;
		}
	}
	else 
	{
		socket_err = SockConnect(sock, ip_by_host, Port);
		if(socket_err == -1) { Err = 1001; return NULL; } // couldn't connect
	}
	
	// делаем запрос
	SMAnsiString get_doc = ((UseSSL)? "https://" : "http://") + HeaderHost + Doc;
	SMAnsiString _ContentReq;
	if (!IsStrEmpty(ContentType))
	{
		_ContentReq.smprintf_s
		(
			"Content-Type: %s\r\n"
			"Content-Length: %llu\r\n",
			ContentType,
			ContentLength
		);
	}
	SMAnsiString req = SMAnsiString::smprintf
	(
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: keep-alive\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"%s" // если есть контент, здесь будет ContentType и ContentLength
		"Sec-Fetch-User: ?1\r\n"
		"Sec-Fetch-Site: none\r\n"
		"Sec-Fetch-Mode: navigate\r\n"
		"Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
		"Accept: */*\r\n\r\n",
		C_STR(get_doc),
		C_STR(HeaderHost),
		C_STR(_ContentReq)
	);

	SMOutBuffer send_msg;
	send_msg.PutBlock(req.c_str(), req.length());
	if (!IsStrEmpty(ContentType)) { send_msg.PutBlock(Content, ContentLength); }

	if(UseSSL)
	{
		socket_err = SSL_write(sock_ssl, send_msg.GetBufferPtr(), send_msg.GetWritePos());
		if(socket_err <= 0) { Err = 1002; return NULL; } // error sending
	}
	else
	{
		socket_err = send(sock, REINTERPRET_CAST_CHARPTR(send_msg.GetBufferPtr()), send_msg.GetWritePos(), 0);
		if(socket_err == -1) { Err = 1002; return NULL; } // error sending
	}
	
	// получаем ответ
	HTTP_Response *resp = (UseSSL) ? RecvResponse(sock_ssl, Err) : nullptr; // TODO: сделать для обычного сокета
	
	// закрываем соединение
	if (UseSSL)
	{
		SSL_shutdown(sock_ssl);
		SSL_free(sock_ssl);
	}
	TGB_CLOSESOCKET(sock);
	
	return resp;
}