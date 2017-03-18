#pragma once
#include "Server.h"
#include "TrayIcon.h"
#include "IconIO.h"
#include "UpdateMessage.h"
#include "cereal\archives\json.hpp"
#include <iostream>
#include <thread>
#include <list>

using namespace std;

TrayIcon* icon;
Server s;

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
				//IconIO::SaveIcon(i->icon, "icona.ico");

				ostringstream ostream;
				{
					cereal::JSONOutputArchive archive(ostream);
					UpdateMessage message(UpdateType::WND_CREATED, GetDlgCtrlID(i->hwnd), i->title);
					message.serialize(archive);
				}
				s.sendMessage(ostream.str().c_str());

				cout << "Created: " << (int)(i->hwnd) << " " << i->title << endl;
				i->isNew = false;
			}

			if (i->isStillRunning) {
				if (i->hwnd == focusedWindow) {
					if (!i->wasFocused) {			//Ha appena ottenuto il focus

						ostringstream ostream;
						{
							cereal::JSONOutputArchive archive(ostream);
							UpdateMessage message(UpdateType::WND_FOCUSED, GetDlgCtrlID(i->hwnd), i->title);
							message.serialize(archive);
						}
						s.sendMessage(ostream.str().c_str());

						cout << "Focused: " << (int)(i->hwnd) << " " << i->title << endl;
						i->wasFocused = true;
					}
				} else
					i->wasFocused = false;
				i->isStillRunning = false;			//Il prossimo ciclo controllerà se è ancora aperta
				++i;
			} else {								//E' stata appena distrutta

				ostringstream ostream;
				{
					cereal::JSONOutputArchive archive(ostream);
					UpdateMessage message(UpdateType::WND_DESTROYED, GetDlgCtrlID(i->hwnd), i->title);
					message.serialize(archive);
				}
				s.sendMessage(ostream.str().c_str());

				cout << "Destroyed: " << (int)(i->hwnd) << " " << i->title << endl;
				i = wList.erase(i);
			}
		}
	}
}

// Processes the messages received by the hidden window
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

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
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main() {
	icon = new TrayIcon(WindowProcedure);
	icon->Show();

	//Avvio Server
	s.startup();

	if (s.listenForClient() == 1)
		cout << "Connected" << endl;

	//Loop enumwin
	thread t(enumLoop, icon->hWnd);

	//Prova deserializzazione

	//{
	//	UpdateMessage message;
	//	istringstream istream(ostream.str());
	//	{
	//		cereal::JSONInputArchive archive(istream);
	//		message.deserialize(archive);
	//	}
	//	cout << "Type: " << message.type << endl << "Title: " << message.wndName << endl << "Id: " << message.wndId << endl;

	//}

	//DLL

	//if (!setDLL())
	//	return -1;

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0)) {
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	t.join();
}