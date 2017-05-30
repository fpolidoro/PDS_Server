#pragma once
#include "Server.h"
#include "TrayIcon.h"
#include "UpdateMessage.h"
#include "KeyMessage.h"
#include "IconManager.h"
#include "cereal\archives\json.hpp"
#include "cereal\external\base64.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>

using namespace std;

TrayIcon* icon;
Server s;
thread server;
bool noFocusSent = false;

atomic_bool connected = false;
atomic_bool quit = false;
mutex mtx;

//Struttura dati per le informazioni di una finestra
typedef struct {
	HWND hwnd;
	HICON icon;
	string title;
	string procName;
	bool isNew;
	bool isStillRunning;
	bool wasFocused;
} WInfo;
//Lista di finestre attive
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

	//Ottiene il path dell'eseguibile relativo alla finestra
	char path[MAX_PATH];
	DWORD value = MAX_PATH;
	PDWORD size = (PDWORD)MAX_PATH;
	DWORD dwProcId = 0;
	GetWindowThreadProcessId(hwnd, &dwProcId);
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, dwProcId);
	QueryFullProcessImageName(hProc, 0, path, &value);
	CloseHandle(hProc);

	//Suddivide il path in drive, directory, nome del file ed estensione
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath_s(path, drive, dir, fname, ext);

	//Filtra le applicazioni di windows 10
	if (string(fname) == "ApplicationFrameHost")
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
	WORD piIcon;
	GetWindowText(hwnd, title, sizeof(title));

	WInfo newWindow;
	newWindow.hwnd = hwnd;
	newWindow.title = "";
	newWindow.title.append(title);
	newWindow.procName.append(fname);
	newWindow.procName.append(ext);
	newWindow.icon = ExtractAssociatedIcon(0, path, &piIcon);
	newWindow.isStillRunning = true;
	newWindow.isNew = true;
	newWindow.wasFocused = false;

	wList.push_front(newWindow);

	return TRUE;
}

//Prepara la stringa JSON
string packJSON(UpdateType type, int hwnd, string title, string procName, string icon) {
	ostringstream ostream;
	try {
		cereal::JSONOutputArchive archive(ostream);
		UpdateMessage message(type, hwnd, title, procName, icon);
		message.serialize(archive);
	} catch (...) {
		cerr << "Error while writing JSON" << endl;
		return "";
	}
	return ostream.str();
}

//Controlla in polling le finestre attive
void enumLoop() {
	while (connected.load()) {
		this_thread::sleep_for(chrono::milliseconds(500));
		lock_guard<mutex> lock(mtx);

		EnumWindows(EnumWindowsProc, NULL);
		HWND focusedWindow = GetForegroundWindow();

		bool foundFocus = false;
		list<WInfo>::iterator i = wList.begin();	//Cicla tutte le finestre
		while (i != wList.end()) {

			if (i->isNew) {							//La finestra è stata appena creata
				string strIcon("");
				IconManager manager;
				BYTE* buf;

				int size = manager.Convert(i->icon, &buf);
				if (size > 0) {
					strIcon = cereal::base64::encode(buf, size);
				}
				delete buf;

				string message = packJSON(UpdateType::WND_CREATED, (int)(i->hwnd), i->title, i->procName, strIcon);
				if (connected.load() && message != "")
					if (s.sendMessage(message.c_str()) != 1) {
						connected.store(false);
						s.close();
						break;
					} else {
						cout << "Created: " << (int)(i->hwnd) << " " << i->title << " " << i->procName << endl;
					}

				i->isNew = false;
			}

			if (i->isStillRunning) {
				if (i->hwnd == focusedWindow) {
					foundFocus = true;
					noFocusSent = false;

					if (!i->wasFocused) {			//La finestra ha appena ottenuto il focus

						string message = packJSON(UpdateType::WND_FOCUSED, (int)(i->hwnd), "", "", "");

						if (connected.load() && message != "")
							if (s.sendMessage(message.c_str()) != 1) {
								connected.store(false);
								s.close();
								break;
							} else {
								cout << "Focused: " << (int)(i->hwnd) << " " << i->title << " " << i->procName << endl;
							}
						i->wasFocused = true;
					}
				} else {
					i->wasFocused = false;
				}

				i->isStillRunning = false;			//Il prossimo ciclo controllerà se la finestra è ancora aperta
				++i;

			} else {								//La finestra è stata appena distrutta

				string message = packJSON(UpdateType::WND_DESTROYED, (int)(i->hwnd), "", "", "");

				if (connected.load() && message != "")
					if (s.sendMessage(message.c_str()) != 1) {
						connected.store(false);
						s.close();
						break;
					} else {
						cout << "Destroyed: " << (int)(i->hwnd) << " " << i->title << " " << i->procName << endl;
					}
				i = wList.erase(i);
			}
		}

		if (!foundFocus && !noFocusSent) {	//Non ci sono finestre in focus e il messaggio per l'assenza del focus non è ancora stato inviato

			noFocusSent = true;
			for each (WInfo i in wList)
				i.wasFocused = false;

			string message = packJSON(UpdateType::WND_FOCUSED, 0, "", "", "");

			if (connected.load() && message != "")
				if (s.sendMessage(message.c_str()) != 1) {
					connected.store(false);
					s.close();
					break;
				} else {
					cout << "No focus" << endl;
				}
		}
	}

	wList.clear();
}

//Mette in focus una finestra e le invia una combinazione di tasti
void SendKeyCombination(HWND hwnd, size_t nKeys, int keys[]) {

	if (hwnd != NULL) {
		ShowWindow(hwnd, SW_RESTORE);
		SetForegroundWindow(hwnd);
	}

	INPUT ip;

	//Effettua il KeyDown per ogni tasto
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = 0;

	for (size_t i = 0; i < nKeys; i++) {
		ip.ki.wVk = keys[i];
		SendInput(1, &ip, sizeof(INPUT));
	}

	//Effettua il KeyUp per ogni tasto
	ip.ki.dwFlags = KEYEVENTF_KEYUP;

	for (size_t i = 0; i < nKeys; i++) {
		ip.ki.wVk = keys[i];
		SendInput(1, &ip, sizeof(INPUT));
	}

	return;
}

void serverLoop() {
	while (!quit.load()) {
		s.startup();
		cout << "Waiting for connection..." << endl;

		if (s.listenForClient() == 1) {
			cout << "Connected" << endl;

			connected.store(true);

			//Loop enumwin e invio
			thread loop(enumLoop);

			//Loop ricezione
			while (connected.load()) {
				std::string str;
				if (s.receiveMessage(str) > 0) {
					KeyMessage message;
					istringstream stream(str);

					try {
						cereal::JSONInputArchive archive(stream);
						message.deserialize(archive);
					} catch (...) {
						cerr << "Error while reading JSON" << endl;
						connected.store(false);
						s.close();
						break;
					}

					if (message.procName != "") {
						lock_guard<mutex> lock(mtx);
						for each (WInfo wInfo in wList) {
							if (wInfo.procName == message.procName) {
								SendKeyCombination(wInfo.hwnd, message.nKeys, message.keys);
							}
						}
					} else {
						SendKeyCombination(GetForegroundWindow(), message.nKeys, message.keys);
					}

				} else {
					connected.store(false);
					s.close();
				}
			}
			loop.join();
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

			SendMessage(hwnd, WM_NULL, 0, 0); // Send benign message to window to make sure the menu goes away.
			if (clicked == ID_TRAY_EXIT) {
				// Quit the application.
				Shell_NotifyIcon(NIM_DELETE, &(icon->iconData));

				connected.store(false);
				quit.store(true);
				s.close();
				server.join();
				PostQuitMessage(0);
			}
		}
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType) {
	connected.store(false);
	quit.store(true);
	s.close();
	server.join();
	return FALSE;
}

int main() {
	SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);

	icon = new TrayIcon(WindowProcedure);
	icon->Show();

	//Connessione e ricezione
	server = thread(serverLoop);

	//Loop messaggi di windows

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0)) {
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
}