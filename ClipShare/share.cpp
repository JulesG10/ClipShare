#include "share.h"

Share::Share()
{
}

Share::~Share()
{
    this->stop();
}

void Share::run_async()
{
    this->m_thread = std::thread([&]() {
        this->run();
    });
}

void Share::run()
{
    LOG("[SHARE]: Started");
    this->m_active = true;

    if (WSAStartup(0x101, &this->m_wsaData) != NO_ERROR)
    {
        LOG("[SHARE]: WSA Startup failed");
        this->m_active = false;
        return;
    }
    LOG("[SHARE]: WSA Startup");

    sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons((u_short)SHARE_PORT);

    this->m_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_server == INVALID_SOCKET)
    {
        LOG("[SHARE]: Create server socket failed");
        this->m_active = false;
        WSACleanup();
        return;
    }
    LOG("[SHARE]: Create server socket");

    if (bind(this->m_server, (sockaddr*)&local, sizeof(local)) != 0)
    {
        LOG("[SHARE]: Bind server socket failed");
        this->m_active = false;
        WSACleanup();
        return;
    }
    LOG("[SHARE]: Bind server socket");
    
    if (listen(this->m_server, 10) != 0)
    {
        LOG("[SHARE]: Listen server socket failed");
        this->m_active = false;
        WSACleanup();
        return;
    }
    LOG("[SHARE]: Listen server socket");

    this->m_client.socket = INVALID_SOCKET;
    this->m_client.addr = { 0 };
    int fromlen = sizeof(this->m_client.addr);

    std::string recieved;

    while (this->m_active)
    {
        if (!this->has_client())
        {
            this->m_client.socket = accept(this->m_server, (struct sockaddr*)&this->m_client.addr, &fromlen);
            LOG("[SHARE]: New client");
        }
        else 
        {
            char buffer[DEFAULT_BUFFER_SIZE];
            int bytesRead = 0;
            if ((bytesRead = recv(this->m_client.socket, buffer, DEFAULT_BUFFER_SIZE, 0)) == SOCKET_ERROR)
            {
                LOG("[SHARE]: Recieve error");
                this->close_client();
            }
            else if (bytesRead == 0)
            {
                LOG("[SHARE]: Client connection closed");
                this->close_client();

                recieved.clear();
            }
            else
            {
                for (size_t i = 0; i < bytesRead; i++)
                {
                    if (buffer[i] == '\r' && i < DEFAULT_BUFFER_SIZE - 3 && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' && buffer[i + 3] == '\n' && std::string(buffer).starts_with("GET /"))
                    {
                        if (this->handle_client_http != NULL)
                        {
                            this->handle_client_http(recieved);
                        }
                        recieved.clear();
                    }
                    else if (buffer[i] == '\r' && (i + 1 < bytesRead && buffer[i + 1] == '\r'))
                    {
                        if (this->handle_client_share != NULL)
                        {
                            this->handle_client_share(ShareData::parse(recieved));
                        }
                        recieved.clear();
                    }
                    else{
                        recieved += buffer[i];
                    }
                }
            }
        }
    }

    this->close_client();
    closesocket(this->m_server);
    LOG("[SHARE]: Close server socket");

    WSACleanup();
    LOG("[SHARE]: WSA Cleanup");
}

void Share::stop()
{
    this->m_active = false;
    if (this->m_thread.joinable())
    {
        this->m_thread.detach();
    }
    LOG("[SHARE]: Stopped");
}

bool Share::has_client()
{
    return this->m_client.socket != INVALID_SOCKET;
}

bool Share::is_active()
{
    return this->m_active;
}

void Share::close_client()
{
    if (this->has_client())
    {
        LOG("[SHARE]: Close client");
        closesocket(this->m_client.socket);
        this->m_client.socket = INVALID_SOCKET;
        this->m_client.addr = { 0 };
    }
}

std::string Share::get_url()
{
    return  this->get_ipv4() + ":" + std::to_string(SHARE_PORT);
}

std::string Share::get_ipv4()
{
    ULONG bufferSize = 0;
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &bufferSize) != ERROR_BUFFER_OVERFLOW) {
        return std::string();
    }

    IP_ADAPTER_ADDRESSES* adapterAddresses = static_cast<IP_ADAPTER_ADDRESSES*>(malloc(bufferSize));
    if (!adapterAddresses) {
        return std::string();
    }

    DWORD result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapterAddresses, &bufferSize);
    if (result != ERROR_SUCCESS) {
        free(adapterAddresses);
        return std::string();
    }

    IP_ADAPTER_ADDRESSES* adapter = adapterAddresses;
    while (adapter) {
        if (adapter->OperStatus == IfOperStatusUp && adapter->Ipv4Enabled) {
            IP_ADAPTER_UNICAST_ADDRESS* unicastAddress = adapter->FirstUnicastAddress;
            while (unicastAddress) {
                sockaddr_in* socketAddress = reinterpret_cast<sockaddr_in*>(unicastAddress->Address.lpSockaddr);
                char ip[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &(socketAddress->sin_addr), ip, INET_ADDRSTRLEN) != nullptr) {
                    free(adapterAddresses);
                    return std::string(ip);
                }
                unicastAddress = unicastAddress->Next;
            }
        }
        adapter = adapter->Next;
    }

    free(adapterAddresses);
    return std::string();
}

void Share::send_share(const ShareData& data)
{
    if (!this->has_client())
    {
        return;
    }

    std::string buff = data.build() + "\r\r";
    if (send(this->m_client.socket, buff.c_str(), (int)buff.size(), NULL) == SOCKET_ERROR)
    {
        LOG("[SHARE]: Send error");
        this->close_client();
    }
}

void Share::send_http_response(const std::string& response)
{
    if (send(this->m_client.socket, response.c_str(), (int)response.size(), NULL) == SOCKET_ERROR)
    {
        LOG("[SHARE]: HTTP send error");
        this->close_client();
    }
}

void Share::send_http_resource(LPCWSTR res_name)
{
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hResource = FindResourceW(hModule, res_name, L"TEXT");

    if (!hResource)
    {
        return;
    }

    HGLOBAL hResourceData = LoadResource(hModule, hResource);
    DWORD resourceSize = SizeofResource(hModule, hResource);

    std::string data;

    if (hResourceData && resourceSize > 0)
    {
        LPVOID pData = LockResource(hResourceData);
        if (pData)
        {
            data = std::string(static_cast<const char*>(pData), resourceSize);
        }
        UnlockResource(hResourceData);
        FreeResource(hResourceData);
    }

    size_t totalSent = 0;
    while (totalSent < data.size())
    {
        size_t remaining = data.size() - totalSent;
        size_t sendSize = min(DEFAULT_BUFFER_SIZE, remaining);

        std::string chunk = data.substr(totalSent, sendSize);

        if (send(this->m_client.socket, chunk.c_str(), (int)chunk.size(), NULL) == SOCKET_ERROR)
        {
            LOG("[SHARE]: HTTP chunk send error");
            this->close_client();
        }
        totalSent += sendSize;
    }
}
