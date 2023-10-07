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

    // Создание сокета UDP
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Настройка адреса и порта
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);                    // Замените на нужный порт
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Замените на IP-адрес сервера

    // Отправка данных
    const char* sendData = "Hi, server!";
    int bytesSent =
        sendto(udpSocket, sendData, strlen(sendData), 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending data." << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::cout << "Data sent successfully." << std::endl;

    // Закрытие сокета и очистка Winsock
    closesocket(udpSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
