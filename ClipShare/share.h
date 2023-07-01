#pragma once

#include "stdafx.h"
#include "resource.h"
#include "share_data.h"

#define SHARE_PORT 20445

typedef struct ShareClient {
	SOCKET socket;
	sockaddr_in addr;
}ShareClient;

class Share
{
public:
	Share();
	~Share();

	void run_async();
	void run();
	void stop();

	bool has_client();
	bool is_active();

	void close_client();

	std::string get_url();
	std::string get_ipv4();

	void send_share(const ShareData&);
	void send_http_response(const std::string&);
	void send_http_resource(LPCWSTR res_name);

	std::function<void(const ShareData&)> handle_client_share = NULL;
	std::function<void(const std::string&)> handle_client_http = NULL;
private:
	bool m_active = false;
	std::thread m_thread;

	SOCKET m_server;
	ShareClient m_client;
	WSADATA m_wsaData;
};

