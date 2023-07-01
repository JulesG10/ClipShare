#pragma once
#include "stdafx.h"

#include "resource.h"

#include "clipboard.h"

#include "qrcode.h"

#include "didec.h"

#include "share.h"
#include "share_data.h"


#define WM_TRAYICON (WM_USER + 1)
#define TRAY_EXIT 1
#define TRAY_PAIR_DEVICE 2
#define TRAY_INSTALL_APP 3

class App
{
public:
	App();
	
	int Start(HINSTANCE hInstance);
	
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static App* Instance()
	{
		if (_instance == nullptr)
		{
			_instance = new App();
		}

		return _instance;
	}
private:
	static App* _instance;

	LRESULT PrivWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void HandleClipChange();
	void HandleClientData(const ShareData&);
	void HandleClientHTTP(const std::string&);

	void RequestFiles(const std::vector<std::string>& files);
	void ReponseFiles(const std::vector<std::vector<std::string>>& files, const int& id);

	void ShowQRCode(std::string prefix_url);

	Share m_share = Share();
	DiDec m_didec = DiDec();
	QRCode m_qrcode = QRCode();

	HWND m_hwnd = NULL;
	HMENU m_hmenu = NULL;
	size_t m_share_id = 0;
	bool m_wait_request = false;
	Path m_qrpath_http;
};

