#include "TrayIcon.h"

// Constructor
TrayIcon::TrayIcon(LRESULT(CALLBACK *WindowProcedure)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)) {

	// Registers the class
	WNDCLASSEX wx = {};
	wx.lpszClassName = CLASSNAME;
	wx.lpfnWndProc = WindowProcedure;
	wx.cbSize = sizeof(WNDCLASSEX);
	if (!RegisterClassEx(&wx))
		exit(-1);

	// Creates an hidden message-only window
	hWnd = CreateWindowEx(0, CLASSNAME, NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

	// Creates the notify iconData
	memset(&iconData, 0, sizeof(NOTIFYICONDATA));
	iconData.cbSize = sizeof(NOTIFYICONDATA);
	iconData.hWnd = hWnd;
	iconData.uID = ID_TRAY_ICON;
	iconData.uCallbackMessage = UWM_TRAYICON; //Set up our invented Windows Message
	iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	iconData.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO));

	HMenu = CreatePopupMenu();
	AppendMenu(HMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
}

// Shows the tray icon
void TrayIcon::Show() {
	Shell_NotifyIcon(NIM_ADD, &iconData);
}

