#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    // Initializing Winsock version 2.2
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Error during initialization Winsock." << std::endl;
        return EXIT_FAILURE;
    }

    // Creating a UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setting the address and port for receiving data
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);  // Replace with the desired port
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Binding a socket to an address and port
    if (bind(udpSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding socket." << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Buffer for receiving data
    char recvBuffer[1024];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    std::cout << "Waiting for data from the client..." << std::endl;

    // Receiving data from the client
    int bytesReceived = recvfrom(
        udpSocket, recvBuffer, sizeof(recvBuffer), 0, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Error while receiving data." << std::endl;
    }
    else
    {
        recvBuffer[bytesReceived] = '\0';
        std::cout << "Received data from client: " << recvBuffer << std::endl;
    }

    // Closing the socket and clearing Winsock
    closesocket(udpSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
