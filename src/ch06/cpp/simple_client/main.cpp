#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    // Инициализация Winsock версии 2.2
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Error during initialization Winsock." << std::endl;
        return EXIT_FAILURE;
    }

    // Создание сокета TCP
    SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Настройка адреса и порта сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);                      // Замените на нужный порт
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Замените на IP-адрес сервера

    // Установка соединения с сервером
    if (WSAConnect(tcpSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr), NULL, NULL, NULL, NULL) ==
        SOCKET_ERROR)
    {
        std::cerr << "Error connecting to server." << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    /*
        Ваш код
    */

    // Закрытие сокета и отчистка Winsock
    closesocket(tcpSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
