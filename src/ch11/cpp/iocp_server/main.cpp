#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>
#include <memory>
#include <thread>
#include <vector>


void worker_thread(HANDLE completion_port)
{
    while (true)
    {
        DWORD bytes_transferred;
        ULONG_PTR completion_key;
        OVERLAPPED* overlapped;
        if (GetQueuedCompletionStatus(completion_port, &bytes_transferred, &completion_key, &overlapped, INFINITE))
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
    const size_t MAX_BUFFER_SIZE = 1024;
    const int PORT = 6250;
    // Initialize Winsock
    socket_wrapper::SocketWrapper sw;

    HANDLE completion_port;
    // Create a listening socket
    auto server_socket = socket_wrapper::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Start several threads to process completed operations
    for (int i = 0; i < 2; ++i)  // For example, two threads are used
    {
        std::thread(worker_thread, completion_port).detach();
    }

    while (true)
    {
        auto client_socket = socket_wrapper::Socket(accept(server_socket, nullptr, nullptr));

        // Create an OVERLAPPED structure for asynchronously receiving data
        std::vector<char> buffer(MAX_BUFFER_SIZE);
        auto overlapped = std::make_unique<OVERLAPPED>();
        DWORD flags = 0;

        // Associate the socket with IOCP
        HANDLE client_handle = reinterpret_cast<HANDLE>(SOCKET(client_socket));

        if (nullptr == CreateIoCompletionPort(client_handle, completion_port, 0, 0))
        {
            std::cerr << sw.get_last_error_string() << std::endl;
            client_socket.close();
            continue;
        }

        // Read data asynchronously
        if (SOCKET_ERROR ==
            WSARecv(client_socket, reinterpret_cast<WSABUF*>(&buffer), 1, nullptr, &flags, overlapped.get(), nullptr))
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                std::cerr << sw.get_last_error_string() << std::endl;
                client_socket.close();
                continue;
            }
        }
    }

    return EXIT_SUCCESS;
}
