#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main() {
    setlocale(0, "Ukrainian");
    system("title SERVER SIDE");

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) return 1;

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res);
    if (result != 0) {
        WSACleanup();
        return 2;
    }

    SOCKET listenSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenSock == INVALID_SOCKET) {
        freeaddrinfo(res);
        WSACleanup();
        return 3;
    }

    result = bind(listenSock, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);
    if (result == SOCKET_ERROR) {
        closesocket(listenSock);
        WSACleanup();
        return 4;
    }

    result = listen(listenSock, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        closesocket(listenSock);
        WSACleanup();
        return 5;
    }

    SOCKET clientSock = accept(listenSock, NULL, NULL);
    if (clientSock == INVALID_SOCKET) {
        closesocket(listenSock);
        WSACleanup();
        return 6;
    }

    closesocket(listenSock);

    char buffer[DEFAULT_BUFLEN];
    int bytesReceived;

    do {
        bytesReceived = recv(clientSock, buffer, DEFAULT_BUFLEN - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';

            const char* reply = "and hello from server!";
            send(clientSock, reply, (int)strlen(reply), 0);
        }
    } while (bytesReceived > 0);

    shutdown(clientSock, SD_SEND);
    closesocket(clientSock);
    WSACleanup();

    return 0;
}
