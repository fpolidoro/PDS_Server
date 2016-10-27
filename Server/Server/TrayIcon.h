#pragma once
#include <windows.h>
#include <shellapi.h>

#define CLASSNAME "ProvaTray"
#define NAME "ProvaTray"

#define UWM_TRAYICON	(WM_USER + 3)

#define ID_TRAY_ICON	1001
#define ID_TRAY_EXIT	1002

#define ICO	101

class TrayIcon {
public:
	TrayIcon(LRESULT(CALLBACK *WindowProcedure)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
	~TrayIcon();
	void Show();
	NOTIFYICONDATA iconData;
	HMENU HMenu;
	HWND hWnd;
};

