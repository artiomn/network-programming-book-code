#include <MSWSock.h>
#include <Winsock2.h>

#include <iostream>
#include <memory>

#define MAX_BUFFER_SIZE 1024

HANDLE completionPort;

void WorkerThread()
{
    while (true)
    {
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        OVERLAPPED* overlapped;
        if (GetQueuedCompletionStatus(completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE))
        {
            // Process completed operation here
            char* buffer = reinterpret_cast<char*>(overlapped);
            std::cout << "Received: " << buffer << std::endl;

            // Perform additional data processing if necessary
        }
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Create a listening socket
    auto serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(6250);

    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Create IOCP
    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completionPort == NULL)
    {
        std::cerr << "Failed to create IOCP." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start several threads to process completed operations
    for (int i = 0; i < 2; i++)
    {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, NULL, 0, NULL);
    }

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        // Create an OVERLAPPED structure for asynchronously receiving data
        std::unique_ptr<char[]> buffer(new char[MAX_BUFFER_SIZE]);
        OVERLAPPED* overlapped = new OVERLAPPED;
        memset(overlapped, 0, sizeof(OVERLAPPED));
        DWORD flags = 0;


        // Associate the socket with IOCP
        if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), completionPort, 0, 0) == NULL)
        {
            std::cerr << "Failed to associate socket with IOCP." << std::endl;
            closesocket(clientSocket);
            delete overlapped;
            continue;
        }

        // Read data asynchronously
        if (WSARecv(clientSocket, reinterpret_cast<WSABUF*>(&buffer), 1, NULL, &flags, overlapped, NULL) ==
            SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                std::cerr << "Failed to initiate asynchronous receive." << std::endl;
                closesocket(clientSocket);
                delete overlapped;
                continue;
            }
        }
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
