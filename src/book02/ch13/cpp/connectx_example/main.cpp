#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <MSWSock.h>
}

#include <cerrno>
#include <iostream>
#include <stdexcept>


LPFN_CONNECTEX load_mswsock()
{
    socket_wrapper::Socket sock(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!sock)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
    }

    GUID guid = WSAID_CONNECTEX;
    DWORD dwBytes;

    LPFN_CONNECTEX result = nullptr;

    if (WSAIoctl(
            sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &result,
            sizeof(result), &dwBytes, nullptr, nullptr) != 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
    }

    return result;
}


int main(int argc, const char *const argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <server> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        socket_wrapper::Socket sock(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        const auto ConnectEx = load_mswsock();
        sockaddr_in local_addr = {.sin_family = AF_INET, .sin_port = 0};
        local_addr.sin_addr.s_addr = INADDR_ANY;

        // ConnectEx requires the socket to be initially bound.
        if (bind(sock, reinterpret_cast<const SOCKADDR *>(&local_addr), sizeof(local_addr)) != 0)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "bind");
        }

        // Call ConnectEx and wait for the operation to complete.
        OVERLAPPED ol = {};
        std::cout << "Connecting to the target: " << argv[1] << ":" << argv[2] << std::endl;
        auto addr = socket_wrapper::get_client_info(argv[1], argv[2]);
        //addr->sin_family = AF_INET;
        //addr->sin_addr.s_addr = inet_addr("64.233.161.101");  // google.com
        //addr->sin_port = htons(80);

        if (ConnectEx(
                sock, reinterpret_cast<const SOCKADDR *>(addr->ai_addr), sizeof(*(addr->ai_addr)), nullptr, 0,
                nullptr, &ol))
        {
            std::cout << "Successfully connected immediately!" << std::endl;
        }
        else
        {
            const auto err_code = WSAGetLastError();

            if (ERROR_IO_PENDING == err_code)
            {
                std::cout << "ConnectEx() in process..." << std::endl;

                DWORD bytes_count;
                if (GetOverlappedResult(reinterpret_cast<HANDLE>(static_cast<SOCKET>(sock)), &ol, &bytes_count, true))
                {
                    std::cout << "ConnectEx() succeeded." << std::endl;
                }
                else
                {
                    // New error code for GetOverlappedResult().
                    throw std::system_error(GetLastError(), std::system_category(), "ConnectEx() failed!");
                }
            }
            else
            {
                throw std::system_error(err_code, std::system_category(), "ConnectEx() failed!");
            }
        }

        if (setsockopt(sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0) != 0)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "SO_UPDATE_CONNECT_CONTEXT failed!");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
