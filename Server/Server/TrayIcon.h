#pragma once
#include <windows.h>
#include <shellapi.h>

#define CLASSNAME "ServerTray"
#define NAME "ServerTray"

#define UWM_TRAYICON	(WM_USER + 3)

#define ID_TRAY_ICON	1001
#define ID_TRAY_EXIT	1002

#define ICO	101

class TrayIcon {
public:
	TrayIcon(LRESULT(CALLBACK *WindowProcedure)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
	void Show();
	NOTIFYICONDATA iconData;
	HMENU HMenu;
	HWND hWnd;
};

