#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main() {
    setlocale(0, "Ukrainian");
    system("title CLIENT SIDE");

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) return 1;

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const char* server = "localhost";
    result = getaddrinfo(server, DEFAULT_PORT, &hints, &res);
    if (result != 0) {
        WSACleanup();
        return 2;
    }

    SOCKET sock = INVALID_SOCKET;
    for (addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            return 3;
        }

        result = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 4;
    }

    const char* msg = "hello from client!";
    result = send(sock, msg, (int)strlen(msg), 0);
    if (result == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return 5;
    }

    shutdown(sock, SD_SEND);

    char buffer[DEFAULT_BUFLEN];
    int bytesReceived;
    do {
        bytesReceived = recv(sock, buffer, DEFAULT_BUFLEN - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << buffer << std::endl;
        }
    } while (bytesReceived > 0);

    closesocket(sock);
    WSACleanup();

    return 0;
}
