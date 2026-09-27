extern "C"
{
#include <MSWSock.h>
#include <WS2tcpip.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cerrno>
#include <iostream>
#include <stdexcept>


struct mswsock_s
{
    LPFN_CONNECTEX ConnectEx;
};


LPFN_CONNECTEX load_mswsock()
{
    socket_wrapper::Socket sock;

    if (!sock)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
    }

    GUID guid = WSAID_CONNECTEX;
    DWORD dwBytes;
    mswsock_s mswsock;

    if (WSAIoctl(
            sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &mswsock.ConnectEx,
            sizeof(mswsock.ConnectEx), &dwBytes, nullptr, nullptr) != 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
    }

    return mswsock.ConnectEx;
}


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <server>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        socket_wrapper::Socket sock;
        const auto ConnectEx = load_mswsock();
        const sockaddr_in local_addr = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = 0};


        // ConnectEx requires the socket to be initially bound.
        if (bind(sock, reinterpret_cast<const SOCKADDR *>(&local_addr), sizeof(local_addr)) != 0)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "bind");
        }

        // Call ConnectEx and wait for the operation to complete.
        OVERLAPPED ol = {};
        const auto addr = socket_wrapper::get_client_info(argv[1], 443);

        if (ConnectEx(sock, reinterpret_cast<const SOCKADDR *>(&addr), sizeof(addr), nullptr, 0, nullptr, &ol))
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
                if (GetOverlappedResult((HANDLE)sock, &ol, &bytes_count, true))
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
