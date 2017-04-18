#pragma once
#include "Server.h"
#include "TrayIcon.h"
#include "IconSave.cpp"
#include "UpdateMessage.h"
#include "KeyMessage.h"
#include "cereal\archives\json.hpp"
#include "cereal\external\base64.hpp"
#include <iostream>
#include <fstream>
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
				IconSave::SaveIcon("temp.ico", &(i->icon), 1);

				ifstream in("temp.ico", ios::binary);
				if (in.is_open()) {
					ostringstream stream;
					stream << in.rdbuf();

					const unsigned char* data = (const unsigned char*)(stream.str().c_str());
					string text = cereal::base64::encode(data, stream.str().length());

					ostringstream ostream;
					{
						cereal::JSONOutputArchive archive(ostream);
						UpdateMessage message(UpdateType::WND_CREATED, (int)(i->hwnd), i->title, text);
						message.serialize(archive);
					}
					s.sendMessage(ostream.str().c_str());
					in.close();
				} else {
					cout << "Error in opening temporary file" << endl;
				}
				cout << "Created: " << (int)(i->hwnd) << " " << i->title << endl;
				i->isNew = false;
			}

			if (i->isStillRunning) {
				if (i->hwnd == focusedWindow) {
					if (!i->wasFocused) {			//Ha appena ottenuto il focus

						ostringstream ostream;
						{
							cereal::JSONOutputArchive archive(ostream);
							UpdateMessage message(UpdateType::WND_FOCUSED, (int)(i->hwnd), i->title, std::string(""));
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
					UpdateMessage message(UpdateType::WND_DESTROYED, (int)(i->hwnd), i->title, std::string(""));
					message.serialize(archive);
				}
				s.sendMessage(ostream.str().c_str());

				cout << "Destroyed: " << (int)(i->hwnd) << " " << i->title << endl;
				i = wList.erase(i);
			}
		}
	}
}

void SendKeyCombinationToTarget(HWND hwnd, size_t nKeys, int keys[]) {
	for (size_t i = 0; i < nKeys; i++) 
		PostMessage(hwnd, WM_KEYDOWN, keys[i], NULL);
	for (size_t i = 0; i < nKeys; i++)
		PostMessage(hwnd, WM_KEYUP, keys[i], NULL);

	cout << "Sent " << keys[0] << " " << keys[1] << " " << keys[2] << " " << keys[3] << " to " << hwnd;

}

void SendKeyCombinationToFocus(size_t nKeys, int keys[]) {

	INPUT ip;

	//Sleep(3000);

	//KeyDown
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = 0;

	for (size_t i = 0; i < nKeys; i++) {
		ip.ki.wVk = keys[i];
		SendInput(1, &ip, sizeof(INPUT));
	}

	//KeyUp
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	for (size_t i = 0; i < nKeys; i++) {
		ip.ki.wVk = keys[i];
		SendInput(1, &ip, sizeof(INPUT));
	}

	return;
}

void serverLoop() {
	if (s.listenForClient() == 1) {
		cout << "Connected" << endl;

		//Loop enumwin
		thread loop(enumLoop, icon->hWnd);

		//Loop ricezione
		while (true) {
			std::string str;
			s.receiveMessage(str);

			KeyMessage message;
			istringstream stream(str);
			{
				cereal::JSONInputArchive archive(stream);
				message.deserialize(archive);
			}
			SendKeyCombinationToFocus(message.nKeys, message.keys);
		}
		loop.join();
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

	//Connessione e ricezione
	thread server(serverLoop);

	//Loop messaggi di windows

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0)) {
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	server.join();
}