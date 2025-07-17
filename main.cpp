#define WIN32_LEAN_AND_MEAN // не тягнем зайвого з windows.h — працює швидше

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") // стандартна бібла для сокетів на Windows

#define DEFAULT_BUFLEN 512 // макс. розмір буфера
#define DEFAULT_PORT "27015" // порт, який будемо слухати

int main() {
    setlocale(0, "Ukrainian");
    system("title SERVER SIDE"); // просто для зручності в терміналі

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); // запускаємо Winsock
    if (result != 0) return 1;

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // бо ми слухаємо, а не ініціюємо

    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res); // отримуємо локальну адресу
    if (result != 0) {
        WSACleanup();
        return 2;
    }

    SOCKET listenSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // створюємо сокет
    if (listenSock == INVALID_SOCKET) {
        freeaddrinfo(res);
        WSACleanup();
        return 3;
    }

    result = bind(listenSock, res->ai_addr, (int)res->ai_addrlen); // прив’язуємо сокет до IP+порт
    freeaddrinfo(res); // вже не треба
    if (result == SOCKET_ERROR) {
        closesocket(listenSock);
        WSACleanup();
        return 4;
    }

    result = listen(listenSock, SOMAXCONN); // починаємо слухати підключення
    if (result == SOCKET_ERROR) {
        closesocket(listenSock);
        WSACleanup();
        return 5;
    }

    SOCKET clientSock = accept(listenSock, NULL, NULL); // чекаємо клієнта
    if (clientSock == INVALID_SOCKET) {
        closesocket(listenSock);
        WSACleanup();
        return 6;
    }

    closesocket(listenSock); // вже не слухаємо — працюємо з клієнтом

    char buffer[DEFAULT_BUFLEN];
    int bytesReceived;

    do {
        bytesReceived = recv(clientSock, buffer, DEFAULT_BUFLEN - 1, 0); // читаємо дані
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // зробили рядок
            const char* reply = "and hello from server!"; // наша відповідь
            send(clientSock, reply, (int)strlen(reply), 0); // відправили
        }
    } while (bytesReceived > 0); // поки щось приходить

    shutdown(clientSock, SD_SEND); // кажемо клієнту: ми закінчили
    closesocket(clientSock); // закрили сокет
    WSACleanup(); // почистили все

    return 0;
}
