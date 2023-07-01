#include "App.h"

App::App()
{
}

int App::Start(HINSTANCE hInstance)
{
    LOG("[APP]: Started");

    WNDCLASS wc{};
    wc.lpfnWndProc = &App::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ClipShare";
    RegisterClassW(&wc);


    LOG("[APP]: Register Class");

    m_hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        wc.lpszClassName,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    

    if (m_hwnd == NULL)
    {
        LOG("[APP]: Create Window failed");
        return 1;
    }
    LOG("[APP]: Create Window");

    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    lstrcpy(nid.szTip, L"Tray Icon Demo");
    Shell_NotifyIcon(NIM_ADD, &nid);

    m_hmenu = CreatePopupMenu();
    AppendMenuA(m_hmenu, MF_STRING, TRAY_PAIR_DEVICE, "Pair Device");
    AppendMenuA(m_hmenu, MF_STRING, TRAY_INSTALL_APP, "Install App");
    AppendMenuA(m_hmenu, MF_STRING, TRAY_EXIT, "Exit");

    LOG("[APP]: TrayMenu");

    this->m_share.handle_client_http = [&](const std::string& request) {
        this->HandleClientHTTP(request);
    };

    this->m_share.handle_client_share = [&](const ShareData& data)
    {
        this->HandleClientData(data);
    };

    this->m_share.run_async();
    LOG("[APP]: TCP Share");

    UpdateWindow(m_hwnd);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    LOG("[APP]: Stop Window");
    this->m_share.stop();
    LOG("[APP]: Stop TCP Share");
    return 1;
}

LRESULT App::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    App* app = App::Instance();
    if (app)
    {
        return app->PrivWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT App::PrivWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND _nextViewer = ClipBoard::Instance()->nextViewer;

    switch (uMsg)
    {
    case WM_CREATE:
        ClipBoard::Instance()->nextViewer = SetClipboardViewer(hwnd);
        break;
    case WM_DESTROY:
        ChangeClipboardChain(hwnd, _nextViewer);
        PostQuitMessage(0);
        break;
    case WM_TRAYICON:
        if (LOWORD(lParam) == WM_RBUTTONUP)
        {
            POINT cursor;
            GetCursorPos(&cursor);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(m_hmenu, TPM_BOTTOMALIGN | TPM_LEFTBUTTON, cursor.x, cursor.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
            break;
        }
        break;
    case WM_PAINT:
        this->m_qrcode.paint(this->m_hwnd, this->m_qrpath_http);
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case TRAY_EXIT:
            DestroyWindow(hwnd);
            break;
        case TRAY_PAIR_DEVICE:
            this->ShowQRCode("clipshare");
            break;
        case TRAY_INSTALL_APP:
            this->ShowQRCode("http");
            break;
        }
        break;
    case WM_DRAWCLIPBOARD:
        this->HandleClipChange();
        break;
    case WM_CHANGECBCHAIN:
        if ((HWND)wParam == _nextViewer)
        {
            _nextViewer = (HWND)lParam;
        }

        if (_nextViewer != NULL)
        {
            SendMessage(_nextViewer, uMsg, wParam, lParam);
        }

        ClipBoard::Instance()->nextViewer = _nextViewer;
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void App::HandleClipChange()
{
    ClipBoardData* clipdata = ClipBoard::Instance()->GetData();
    
    ShareData shareData = ShareData();
    shareData.files = clipdata->files;
    shareData.text = clipdata->text;
    shareData.id = this->m_share_id++;
    
    this->m_share.send_share(shareData);

    delete clipdata;
}

void App::HandleClientData(const ShareData& data)
{
    this->RequestFiles(data.request_files);
    this->ReponseFiles(data.response_files, data.id);
}

void App::HandleClientHTTP(const std::string&)
{
    if (this->m_wait_request)
    {
        this->m_wait_request = false;
        ShowWindow(this->m_hwnd, SW_HIDE);
        UpdateWindow(this->m_hwnd);
    }
    
    std::string filename = "clipshare.apk";
    this->m_share.send_http_response("HTTP/1.0 200 OK\r\nContent-Disposition: attachment; filename=" + filename + "\r\nContent-Type: application/octet-stream\r\n\r\n");
    this->m_share.send_http_resource(MAKEINTRESOURCE(IDR_TEXT1));
    this->m_share.close_client();
}

void App::RequestFiles(const std::vector<std::string>& files)
{
    if (files.size() == 0)
    {
        return;
    }

    std::vector<std::vector<std::string>> response_files = {};

    for (std::string item : files)
    {
        Path itempath = Path(item);
        if (file_exists(itempath))
        {
            std::ifstream file(itempath);
            if (file.good())
            {
                std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                response_files.push_back({ itempath, content });
            }

            file.close();
        }
        else if (dir_exists(itempath))
        {
            Path tmp_dirfile = get_temp_path().append(sha1(itempath.get_name()) + ".dir");

            if (this->m_didec.encode_directory(itempath, tmp_dirfile))
            {
                std::ifstream file(tmp_dirfile);
                if (file.good())
                {
                    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                    response_files.push_back({ itempath, content});
                }

                file.close();
                DeleteFileA((LPCSTR)tmp_dirfile);
            }          
        }
    }

    ShareData shareData;
    shareData.id = this->m_share_id++;
    shareData.response_files = response_files;
    this->m_share.send_share(shareData);
}

void App::ReponseFiles(const std::vector<std::vector<std::string>>& files,const int& id)
{
    if (files.size() == 0)
    {
        return;
    }

    Path clipshare_dir;

    clipshare_dir = get_temp_path();
    if (!create_if_nexists(clipshare_dir))
    {
        return;
    }

    Path id_dir = clipshare_dir + Path(std::to_string(id)).to_dir();
    if (!create_if_nexists(id_dir))
    {
        return;
    }

    for (std::vector<std::string> vec : files)
    {
        if (vec.size() == 2)
        {
            Path itempath = Path(vec[0]);
            std::string content = vec[1];

            if (itempath.is_dir())
            {
                Path new_dir = id_dir.copy().append(itempath.get_name(), false);
                Path tmp_dirfile = new_dir + Path(".dir");

                std::ofstream file((std::string)tmp_dirfile, std::ios::out | std::ios::binary);
                if (file.good())
                {
                    file << content;
                }
                file.close();
                
                create_if_nexists(new_dir);
                this->m_didec.decode_directory(tmp_dirfile, (std::string)new_dir + "\\");
                DeleteFileA((LPCSTR)tmp_dirfile);
            }
            else 
            {
                std::string new_filepath = id_dir.copy().append(itempath.get_name(), false);

                std::ofstream file(new_filepath, std::ios::out | std::ios::binary);
                if (file.good())
                {
                    file << content;
                }
                file.close();
            }
        }
       
    }
}

void App::ShowQRCode(std::string prefix_url)
{
    this->m_qrpath_http = get_temp_path();
    this->m_qrpath_http.append(prefix_url +"_clipshare_qrcode.bmp");

    int winSize = this->m_qrcode.generate(prefix_url +"://" + this->m_share.get_url() + "/", this->m_qrpath_http);
    this->m_wait_request = true;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    SetWindowPos(this->m_hwnd,
        NULL,
        (screenWidth - winSize) / 2,
        (screenHeight - winSize) / 2,
        winSize,
        winSize,
        SWP_NOZORDER | SWP_NOMOVE);

    ShowWindow(this->m_hwnd, SW_SHOW);
    UpdateWindow(this->m_hwnd);
}


App* App::_instance = nullptr;