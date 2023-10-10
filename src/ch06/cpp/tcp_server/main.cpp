#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>

int main()
{
    // Initializing Winsock version 2.2
    socket_wrapper::SocketWrapper sw;
    /*WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Error during initialization Winsock." << std::endl;
        return EXIT_FAILURE;
    }*/

    // Creating a TCP socket
    auto listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setting the address and port for listening
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6250);        // Replace with server port
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // We accept connections on all available network interfaces

    // Binding a socket to an address and port
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Listening to a socket for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Error while listening on socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::cout << "Server is waiting for connection..." << std::endl;

    // Accepting incoming connections
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Server is waiting for connection." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::cout << "Connection established." << std::endl;

    // Example of reading data from a client
    char recvBuffer[1024];
    int bytesRead = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesRead > 0)
    {
        recvBuffer[bytesRead] = '\0';
        std::cout << "Received data from client: " << recvBuffer << std::endl;
    }
    else if (bytesRead == 0)
    {
        std::cout << "Connection closed by client." << std::endl;
    }
    else
    {
        std::cerr << "Error reading data." << std::endl;
    }

    // Closing sockets and clearing Winsock
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return EXIT_SUCCESS;
}
