#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

#define PORT "27015"
#define BUFLEN 512

class Server {
public:
	Server();
	~Server();
	int startup();
	int listenForClient();
	int close();
	int sendMessage(const char* str);
	int receiveMessage(std::string& str);

private:
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
	struct addrinfo *addressinfo = NULL;
};



