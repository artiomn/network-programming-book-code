// #include <winsock2.h>
// #include <ws2tcpip.h>

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>

int main()
{
    // Initializing Winsock version 2.2
    socket_wrapper::SocketWrapper sw;
    // WSADATA wsaData;
    // if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    //{
    //     std::cerr << "Error during initialization Winsock." << std::endl;
    //     return EXIT_FAILURE;
    // }

    // Creating a UDP socket
    auto udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setting the address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);                    // Replace with server port
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Replace with server IP address

    // Sending data
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

    // Closing sockets and clearing Winsock
    closesocket(udpSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
