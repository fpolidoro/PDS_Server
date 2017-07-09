#pragma once
#include "Server.h"

Server::Server() {}

Server::~Server() {
	close();
}

int Server::startup() {

	struct addrinfo hints;

	// Initialize Winsock
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cout << "Failed to initialize Winsock, error: " << WSAGetLastError() << std::endl;
		return WSAGetLastError();
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	result = getaddrinfo(NULL, PORT, &hints, &addressinfo);
	if (result != 0) {
		std::cout << "Failed to resolve address or port, error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return WSAGetLastError();
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(addressinfo->ai_family, addressinfo->ai_socktype, addressinfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Failed to create a socket, error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addressinfo);
		WSACleanup();
		return WSAGetLastError();
	}

	// Setup the TCP listening socket
	result = bind(ListenSocket, addressinfo->ai_addr, (int)addressinfo->ai_addrlen);
	if (result == SOCKET_ERROR) {
		std::cout << "Failed to setup the listening socket, error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addressinfo);
		closesocket(ListenSocket);
		WSACleanup();
		return WSAGetLastError();
	}
	freeaddrinfo(addressinfo);

	return 1;
}

int Server::listenForClient() {

	int result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		std::cout << "Failed to listen, error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Failed to accept client, error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	return 1;
}

int Server::sendMessage(const char* str) {

	//Prepara un prefisso di 4 byte per la dimensione del messaggio
	INT32 length = strlen(str) + 1;
	char* buf = new char[length + 4];

	buf[0] = length & 0xff;
	buf[1] = (length >> 8) & 0xff;
	buf[2] = (length >> 16) & 0xff;
	buf[3] = (length >> 24) & 0xff;

	//Accoda il messaggo
	memcpy(buf + 4, str, length);

	//Invia
	int result = send(ClientSocket, buf, length + 4, 0);
	if (result == SOCKET_ERROR) {
		std::cout << "Send failed, error: " << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	delete[] buf;
	return 1;
}

int Server::receiveMessage(std::string& str) {

	//Leggi i primi 4 byte del messaggio per ottenerne la dimensione
	char* buf = new char[4];
	int result = recv(ClientSocket, buf, 4, 0);

	if (result == SOCKET_ERROR || result == 0) {
		std::cout << "Receive failed, error: " << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return WSAGetLastError();
	}
	int lenght = *(int*) buf;
	delete[] buf;

	//Ora che la dimensione è nota, leggi il messaggio
	buf = new char[lenght];
	result = recv(ClientSocket, buf, lenght, 0);
	if (result == SOCKET_ERROR || result == 0) {
		std::cout << "Receive failed, error: " << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	std::string received(buf);
	str.append(received);

	delete[] buf;
	return 1;
}

int Server::close() {

	int result = shutdown(ClientSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		std::cout << "Shutdown failed, error: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		closesocket(ClientSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();
	return 1;
}
