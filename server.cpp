#define WIN32_LEAN_AND_MEAN // макрос, шоб при імпорті <windows.h> не тягнути зайве — пришвидшує збірку

#include <iostream> // стандартна бібліотека — дає нам cout, cin і тд
#include <windows.h> // потрібен для Sleep(), system() і всяких WinAPI штук
#include <ws2tcpip.h> // сюди входять структури, функції і типи для TCP/IP роботи на Win

#pragma comment(lib, "Ws2_32.lib") // це лінкеру кажемо підключити бібліотеку для сокетів
#pragma comment(lib, "Mswsock.lib") // для розширених сокетів (на всяк випадок)
#pragma comment(lib, "AdvApi32.lib") // API для деяких безпечних операцій (нам не критично, але ок)

#define DEFAULT_BUFLEN 512 // розмір буфера для прийому/відправки (512 байт)
#define DEFAULT_PORT "27015" // порт, на якому будемо слухати/підключатись

int main() {
    setlocale(0, "Ukrainian"); // шоб українські символи в терміналі норм відображались
    system("title CLIENT SIDE"); // назва вікна терміналу (для зручності, типу клієнт видно)

    WSADATA wsaData; // структура, куди запишеться інфа про версію Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); // ініціалізація бібліотеки сокетів
    if (result != 0) return 1; // якщо не 0 — значить шось пішло не так, виходимо

    addrinfo hints{}, *res = nullptr; // hints — що саме шукаємо, res — результат
    hints.ai_family = AF_UNSPEC; // можна і IPv4, і IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP-сокет
    hints.ai_protocol = IPPROTO_TCP; // явно TCP (хоч і так зрозуміло по типу)

    const char* server = "localhost"; // будемо підключатись до себе (на локалку)
    result = getaddrinfo(server, DEFAULT_PORT, &hints, &res); // отримати дані для підключення
    if (result != 0) {
        WSACleanup(); // прибрати за собою
        return 2;
    }

    SOCKET sock = INVALID_SOCKET; // сокет, через який будемо з'єднуватись
    for (addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next) { // пробігаємось по можливих адресах
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // створюємо сокет
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            return 3;
        }

        result = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen); // пробуємо підключитись
        if (result == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue; // пробуємо наступну адресу
        }
        break; // успішне з'єднання
    }

    freeaddrinfo(res); // вже не треба — звільняємо пам’ять

    if (sock == INVALID_SOCKET) { // якщо все ще не з’єднались — біда
        WSACleanup();
        return 4;
    }

    const char* msg = "hello from client!"; // повідомлення, яке відправимо на сервер
    result = send(sock, msg, (int)strlen(msg), 0); // відправка
    if (result == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return 5;
    }

    shutdown(sock, SD_SEND); // кажемо: все, більше нічого не шлемо

    char buffer[DEFAULT_BUFLEN]; // буфер для прийому
    int bytesReceived;
    do {
        bytesReceived = recv(sock, buffer, DEFAULT_BUFLEN - 1, 0); // читаємо, що сервер відповів
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // додаємо нуль, щоб був правильний рядок
            std::cout << buffer << std::endl; // виводимо відповідь
        }
    } while (bytesReceived > 0); // поки є дані

    closesocket(sock); // закриваємо сокет
    WSACleanup(); // чистимо все, що ініціалізували

    return 0;
}
