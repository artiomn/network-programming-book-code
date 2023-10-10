#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>

int main()
{
    // Initializing Winsock version 2.2
    socket_wrapper::SocketWrapper sw;
    /* WSADATA wsaData;
     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
     {
         std::cerr << "Error during initialization Winsock." << std::endl;
         return EXIT_FAILURE;
     }*/

    // Creating a TCP socket
    auto tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setting the server address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);                      // Replace with server port
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Replace with server IP address

    // Establishing a connection to the server
    if (WSAConnect(tcpSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr), NULL, NULL, NULL, NULL) ==
        SOCKET_ERROR)
    {
        std::cerr << "Error connecting to server." << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    /*
        Your code
    */

    // Closing sockets and clearing Winsock
    closesocket(tcpSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
