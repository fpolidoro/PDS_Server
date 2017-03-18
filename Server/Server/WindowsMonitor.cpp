#pragma once
#include "TrayIcon.h"
#include "IconIO.h"
#include "rapidjson\prettywriter.h"
#include "Messages.h"
#include <iostream>
#include <thread>
#include <list>

#define UWM_WCREATE		(WM_USER + 0)
#define UWM_WDESTROYED	(WM_USER + 1)
#define UWM_WFOCUSED	(WM_USER + 2)

using namespace std;
using namespace rapidjson;

TrayIcon* icon;

typedef struct {
	HWND hwnd;
	HICON icon;
	char title[100];
	bool isNew;
	bool isStillRunning;
	bool wasFocused;
} WInfo;
list<WInfo> wList;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	//Filtra le finestre
	TITLEBARINFO ti;
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return TRUE;

	if (!IsWindowVisible(hwnd) || (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
		return TRUE;

	//La finestra è valida

	list<WInfo>::iterator i = wList.begin();
	while (i != wList.end()) {
		if (i->hwnd == hwnd) {
			i->isStillRunning = true;	//La finestra è ancora in lista
			return TRUE;
		}
		++i;
	}

	//La finestra è nuova
	char title[100];
	GetWindowText(hwnd, title, sizeof(title));

	WInfo newWindow;
	newWindow.hwnd = hwnd;
	strcpy_s(newWindow.title, sizeof(title), title);
	newWindow.icon = (HICON)GetClassLong(hwnd, GCL_HICON);
	newWindow.isStillRunning = true;
	newWindow.isNew = true;
	newWindow.wasFocused = false;

	wList.push_front(newWindow);

	//char class_name[MAX_LENGHT];
	//GetClassName(hwnd, class_name, sizeof(class_name));

	return TRUE;
}

void enumLoop(HWND hWnd) {

	while (true) {
		this_thread::sleep_for(chrono::milliseconds(500));

		EnumWindows(EnumWindowsProc, NULL);
		HWND focusedWindow = GetForegroundWindow();

		list<WInfo>::iterator i = wList.begin();
		while (i != wList.end()) {
			if (i->isNew) {							//E' stata appena creata
				PostMessage(hWnd, UWM_WCREATE, (WPARAM)i->title, NULL);

				IconIO::SaveIcon(i->icon, "icona.ico");

				i->isNew = false;
			}

			if (i->isStillRunning) {
				if (i->hwnd == focusedWindow) {
					if (!i->wasFocused) {			//Ha appena ottenuto il focus
						PostMessage(hWnd, UWM_WFOCUSED, (WPARAM)i->title, NULL);
						i->wasFocused = true;
					}
				} else
					i->wasFocused = false;
				i->isStillRunning = false;			//Il prossimo ciclo controllerà se è ancora aperta
				++i;
			} else {									//E' stata appena distrutta
				char temp[100];
				strcpy_s(temp, sizeof(i->title), i->title);
				PostMessage(hWnd, UWM_WDESTROYED, (WPARAM)temp, NULL);
				i = wList.erase(i);
			}
		}
	}
}

// Processes the messages received by the hidden window
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	LPSTR title = "";
	switch (message) {
	case UWM_TRAYICON:

		if (wParam == ID_TRAY_ICON)
			SetForegroundWindow(hwnd);

		if (lParam == WM_RBUTTONDOWN) {
			// Get current mouse position.
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns
			UINT clicked = TrackPopupMenu(icon->HMenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked == ID_TRAY_EXIT) {
				// quit the application.
				Shell_NotifyIcon(NIM_DELETE, &(icon->iconData));
				PostQuitMessage(0);
			}
		}
		break;
	case UWM_WCREATE:
		title = (LPSTR)wParam;
		cout << "Created: " << title << endl;
		break;
	case UWM_WDESTROYED:
		title = (LPSTR)wParam;
		cout << "Destroyed: " << title << endl;
		break;
	case UWM_WFOCUSED:
		title = (LPSTR)wParam;
		cout << "Focused: " << title << endl;
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main() {
	icon = new TrayIcon(WindowProcedure);
	icon->Show();

	//HWND hwnd = GetForegroundWindow();
	//WindowCreated wc1;
	//GetWindowText(GetForegroundWindow(), wc1.title, 100);
	//wc1.id = GetDlgCtrlID(hwnd);

	//cout << "Title: " << wc1.title << endl << "Id: " << wc1.id << endl;

	//StringBuffer sb;
	//PrettyWriter<StringBuffer> writer(sb);
	//wc1.Serialize(writer);
	////char* str = (char*)malloc(sb.GetLength() * sizeof(char));
	////strcpy_s(str, sb.GetLength(), sb.GetString());
	//cout << sb.GetString() << endl;

	//Document d;
	//if (!d.Parse(str).HasParseError())
	//	cout << "error" << endl;
	//else {
	//	WindowCreated wc2;
	//	wc2.Deserialize(d);
	//	cout << "Title: " << wc2.title << endl << "Id: " << wc2.id << endl;
	//}

	//if (!setDLL())
	//	return -1;

	//Server s;
	//s.startup();
	//if (s.listenForClient() == 1)
	//	std::cout << "Connected" << std::endl;

	thread t(enumLoop, icon->hWnd);

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0)) {
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	//t.join();
}