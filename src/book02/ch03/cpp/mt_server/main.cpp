#if defined(WIN32)
#    define NOMINMAX
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if !defined(_WIN32)
extern "C"
{
#    include <signal.h>
}
#else
#    include <cwctype>
#endif

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>


#if !defined(MAX_PATH)
#    define MAX_PATH 256
#endif


constexpr auto clients_count = 10;
constexpr auto buffer_size = 4096;
using namespace std::literals;
namespace fs = std::filesystem;

#if defined(_WIN32)
const wchar_t separ = fs::path::preferred_separator;
#else
const wchar_t separ = *reinterpret_cast<const wchar_t *>(&fs::path::preferred_separator);
#endif


class Transceiver
{
public:
    explicit Transceiver(socket_wrapper::Socket &&client_sock) : client_sock_(std::move(client_sock)) {}
    explicit Transceiver(const Transceiver &) = delete;
    Transceiver() = delete;

public:
    const socket_wrapper::Socket &ts_socket() const { return client_sock_; }

public:
    bool send_buffer(const std::vector<char> &buffer)
    {
        size_t transmit_bytes_count = 0;
        const auto size = buffer.size();

        while (transmit_bytes_count != size)
        {
            auto result =
                send(client_sock_, &(buffer.data()[0]) + transmit_bytes_count, size - transmit_bytes_count, 0);
            if (-1 == result)
            {
                if (need_to_repeat()) continue;
                return false;
            }

            transmit_bytes_count += result;
        }

        return true;
    }

    bool send_file(fs::path const &file_path)
    {
        std::vector<char> buffer(buffer_size);
        std::ifstream file_stream(file_path, std::ifstream::binary);

        if (!file_stream) return false;

        std::cout << "Sending file " << file_path << "..." << std::endl;
        while (file_stream)
        {
            file_stream.read(&buffer[0], buffer.size());
            if (!send_buffer(buffer)) return false;
        }

        return true;
    }

    std::string get_request()
    {
        std::array<char, MAX_PATH + 1> buffer;
        size_t recv_bytes = 0;
        const auto size = buffer.size() - 1;

        std::cout << "Reading user request..." << std::endl;
        while (true)
        {
            auto result = recv(client_sock_, &buffer[recv_bytes], size - recv_bytes, 0);

            if (!result) break;

            if (-1 == result)
            {
                if (need_to_repeat()) continue;
                throw std::system_error(errno, std::system_category(), "Socket reading error");
            }

            auto fragment_begin = buffer.begin() + recv_bytes;
            auto ret_iter = std::find_if(
                fragment_begin, fragment_begin + result, [](char sym) { return '\n' == sym || '\r' == sym; });
            if (ret_iter != buffer.end())
            {
                *ret_iter = '\0';
                recv_bytes += std::distance(fragment_begin, ret_iter);
                break;
            }
            recv_bytes += result;
            if (size <= recv_bytes) break;
        }

        buffer[std::min(size, recv_bytes)] = '\0';

        auto result = std::string(buffer.begin(), buffer.begin() + recv_bytes);
        std::cout << "Request = \"" << result << "\"" << std::endl;

        return result;
    }

private:
    static bool need_to_repeat()
    {
        switch (errno)
        {
            // These are not errors: must to repeat send().
            case EINTR:
            case EWOULDBLOCK:
#if defined(EAGAIN)
#    if (EAGAIN) != (EWOULDBLOCK)
            case EAGAIN:
#    endif
#endif
                return true;
        }

        return false;
    };

private:
    socket_wrapper::Socket client_sock_;
};


class Client
{
public:
    explicit Client(socket_wrapper::Socket &&sock) : tsr_(std::move(sock))
    {
        std::cout << "Client [" << static_cast<int>(tsr_.ts_socket()) << "] "
                  << "was created..." << std::endl;
    }

    std::optional<fs::path> recv_file_path()
    {
        auto request_data = tsr_.get_request();
        if (!request_data.size()) return std::nullopt;

        auto cur_path = fs::current_path().wstring();
        auto file_path = fs::weakly_canonical(request_data).wstring();

#if defined(_WIN32)
        std::transform(cur_path.begin(), cur_path.end(), cur_path.begin(), [](wchar_t c) { return std::towlower(c); });
        std::transform(
            file_path.begin(), file_path.end(), file_path.begin(), [](wchar_t c) { return std::towlower(c); });
#endif
        if (file_path.find(cur_path) == 0)
        {
            file_path = file_path.substr(cur_path.length());
        }

        return fs::weakly_canonical(cur_path + separ + file_path);
    }

    bool send_file(const fs::path &file_path)
    {
        if (!(fs::exists(file_path) && fs::is_regular_file(file_path))) return false;

        return tsr_.send_file(file_path);
    }

    bool process()
    {
        auto file_to_send = recv_file_path();
        bool result = false;

        if (std::nullopt != file_to_send)
        {
            std::cout << "Trying to send " << *file_to_send << "..." << std::endl;
            if (send_file(*file_to_send))
            {
                std::cout << "File was sent." << std::endl;
            }
            else
            {
                std::cerr << "File sending error!" << std::endl;
            }
            result = true;
        }

        return result;
    }

private:
    Transceiver tsr_;
};


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

#if !defined(_WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = std::move(socket_wrapper::get_serv_info(argv[1]));

        socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (!server_sock)
        {
            throw std::system_error(errno, std::system_category(), "Socket creation error");
        }

        socket_wrapper::set_reuse_addr(server_sock);

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(errno, std::system_category(), "Bind error");
        }

        std::list<std::future<bool>> pending_tasks;

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::system_error(errno, std::system_category(), "Listen error");
        }

        std::cout << "Listening on port " << argv[1] << "...\n"
                  << "Server path: " << fs::current_path() << std::endl;

        while (true)
        {
            auto client_sock = socket_wrapper::accept_client(server_sock);

            pending_tasks.push_back(std::async(
                std::launch::async,
                [&](socket_wrapper::Socket &&sock)
                {
                    Client client(std::move(sock));
                    std::cout << "Client tid = " << std::this_thread::get_id() << std::endl;
                    auto result = client.process();
                    std::cout << "Client with tid = " << std::this_thread::get_id() << " exiting..." << std::endl;

                    return result;
                },
                std::move(client_sock)));

            std::cout << "Cleaning tasks..." << std::endl;
            for (auto task = pending_tasks.begin(); task != pending_tasks.end();)
            {
                if (std::future_status::ready == task->wait_for(1ms))
                {
                    auto fu = task++;
                    std::cout << "Request completed with a result = " << fu->get() << "...\n"
                              << "Removing from list." << std::endl;
                    pending_tasks.erase(fu);
                }
                else
                    ++task;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
