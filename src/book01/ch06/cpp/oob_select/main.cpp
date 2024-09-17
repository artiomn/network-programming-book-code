extern "C"
{
#include <fcntl.h>
#include <sys/ioctl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <array>
#include <cassert>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


constexpr size_t buffer_size = 255;

void set_nonblock(SocketDescriptorType fd)
{
    const IoctlType flag = 1;

    if (ioctl(fd, FIONBIO, const_cast<IoctlType *>(&flag)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Setting non-blocking mode");
    }
}


void recv_data(
    const socket_wrapper::SocketWrapper &sock_wrap, const socket_wrapper::Socket &client_sock,
    std::array<char, buffer_size> &data_buff)
{
    if (ssize_t n = recv(client_sock, data_buff.data(), data_buff.size(), 0); n < 0)
    {
        if (auto e_code = sock_wrap.get_last_error_code(); EINTR == e_code || EAGAIN == e_code)
        {
            std::cout << "recv was interrupted!" << std::endl;
        }
        else
            throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "recv data");
    }
    else if (!n)
    {
        std::cout << "No data, exiting..." << std::endl;
        exit(EXIT_SUCCESS);
    }
    else
    {
        std::cout << "Ordinary data received...\n"
                  << n << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n) << std::endl;
    }
}


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    const socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        assert(argv[1]);
        auto server_sock = socket_wrapper::create_tcp_server(argv[1]);
        auto client_sock = socket_wrapper::accept_client(server_sock);

        set_nonblock(client_sock);

        fd_set read_descriptors_set;
        // fd_set write_descriptors_set;
        fd_set err_descriptors_set;

        std::array<char, buffer_size> data_buff;

        while (true)
        {
            timeval timeout = {.tv_sec = 1, .tv_usec = 0};
            FD_ZERO(&err_descriptors_set);
            FD_ZERO(&read_descriptors_set);
            FD_SET(client_sock, &err_descriptors_set);
            FD_SET(client_sock, &read_descriptors_set);

            auto active_descriptors = select(
                client_sock + 1, &read_descriptors_set, nullptr /* &write_descriptors_set */, &err_descriptors_set,
                &timeout);

            if (active_descriptors < 0)
            {
                throw std::system_error(errno, std::system_category(), "select");
            }

            if (0 == active_descriptors)
            {
                // Nothing to do.
                std::cout << "No active descriptors..." << std::endl;
                continue;
            }

            if (FD_ISSET(client_sock, &err_descriptors_set))
            {
                std::cout << "OOB or error" << std::endl;
                switch (sockatmark(client_sock))
                {
                    case -1:
                        throw std::system_error(errno, std::system_category(), "sockatmark");
                        break;
                    case 1:
                        std::cout << "OOB data received..?" << std::endl;
                        if (char oob_data = 0; -1 == recv(client_sock, &oob_data, 1, MSG_OOB))
                        {
                            if (EINVAL == sock_wrap.get_last_error_code())
                            {
                                std::cout << "EINVAL - this is not OOB" << std::endl;
                                recv_data(sock_wrap, client_sock, data_buff);
                                continue;
                            }
                            throw std::system_error(
                                sock_wrap.get_last_error_code(), std::system_category(), "recv oob");
                        }
                        else
                            std::cout << "OOB data = " << oob_data << std::endl;
                        break;
                    case 0:
                        std::cout << "sockatmark() is 0" << std::endl;
                        break;
                    default:
                        throw std::runtime_error("unexpected sockatmark");
                }
            }

            if (FD_ISSET(client_sock, &read_descriptors_set))
            {
                recv_data(sock_wrap, client_sock, data_buff);
                continue;
            }
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
