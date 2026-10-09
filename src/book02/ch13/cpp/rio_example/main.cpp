#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <MSWSock.h>
}

#include <array>
#include <iostream>
#include <system_error>

#include "rio_interface.h"


constexpr auto server_port = 8080;


int main()
{
    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        socket_wrapper::Socket server_sock(RIO::make_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        RIO rio(server_sock);

        sockaddr_in local_addr
        {
            .sin_family = AF_INET,
            .sin_port = htons(server_port)
        };

        local_addr.sin_addr.s_addr = INADDR_ANY;

        if (SOCKET_ERROR == bind(server_sock, reinterpret_cast<sockaddr *>(&local_addr), sizeof(local_addr)))
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "bind()");
        }

        if (SOCKET_ERROR == listen(server_sock, SOMAXCONN))
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "listen()");
        }

        std::cout << "Server was started on port " << server_port << std::endl;

        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);
        socket_wrapper::Socket client_sock(accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len));

        if (INVALID_SOCKET == client_sock)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "accept()");
        }

        std::cout << "Client was connected..." << std::endl;

        auto [client_queue, raw_buffer] = rio.init_data_exchange(client_sock);
        auto iocp_handle = rio.get_iocp();

        std::array<RIORESULT, RIO::rio_max_results> results;
        bool done = false;

        // Notification.
        if (rio.notify(client_queue) != ERROR_SUCCESS)
        {
            done = true;
        }

        while (!done)
        {
            DWORD bytesCount = 0;
            ULONG_PTR completionKey = 0;
            LPOVERLAPPED pOverlapped = nullptr;

            // Sleep until RIO trigger will be in the signaled state.
            if (!GetQueuedCompletionStatus(iocp_handle, &bytesCount, &completionKey, &pOverlapped, INFINITE))
            {
                throw std::system_error(GetLastError(), std::system_category(), "IOCP Error");
            }

            // Data reading from the RIO queue.
            while (!done)
            {
                auto results_count = rio.read_completion_queue(client_queue, results);

                if (0 == results_count)
                {
                    // Empty queue, enable notification trigger.
                    if (rio.notify(client_queue) != ERROR_SUCCESS)
                    {
                        done = true;
                        break;
                    }

                    // If the packet was received during RIONotify() call,
                    // get it now, otherwise GetQueuedCompletionStatus() will hang on the next loop iteration.
                    results_count = rio.read_completion_queue(client_queue, results);
                    if (0 == results_count)
                    {
                        // No data.
                        break;
                    }
                }

                for (ULONG i = 0; i < results_count; ++i)
                {
                    // if RequestContext is 0, sending was completed.
                    // Free network adapter and ignore this.
                    if (0 == results[i].RequestContext)
                    {
                        if (results[i].Status != 0)
                        {
                            std::cerr << "RIO async sending error: " << results[i].Status << std::endl;
                        }
                        continue;
                    }

                    // Receiving operation.
                    RIO::EXTENDED_RIO_BUF* buffer_ptr = reinterpret_cast<RIO::EXTENDED_RIO_BUF*>(results[i].RequestContext);
                    if (0 == results[i].Status)
                    {
                        ULONG bytes_transferred = results[i].BytesTransferred;
                        if (0 == bytes_transferred)
                        {
                            std::cout << "Client was disconnected..." << std::endl;
                            done = true;
                            break;
                        }
                        char *packet_data = raw_buffer.get() + buffer_ptr->Offset;
                        std::cout
                            << "Data received (" << bytes_transferred << " bytes): "
                            << std::string(packet_data, bytes_transferred)
                            << std::endl;
                        // Copy data from the recieving buffer and send it back.
                        RIO::EXTENDED_RIO_BUF* send_buffer_ptr= rio.get_send_buf(buffer_ptr->operation_id_);
                        char *send_data_ptr = raw_buffer.get() + send_buffer_ptr->Offset;
                        std::copy(packet_data, packet_data + bytes_transferred, send_data_ptr);
                        send_buffer_ptr->Length = bytes_transferred;
                        // Sending context is 0.
                        rio.send(send_buffer_ptr, 1, 0, 0);
                        // Return receving buffer.
                        rio.recv(buffer_ptr, 1, 0, buffer_ptr);
                    }
                    else
                    {
                        std::cerr << "RIO recieving error: " << results[i].Status << std::endl;
                        done = true;
                        break;
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}