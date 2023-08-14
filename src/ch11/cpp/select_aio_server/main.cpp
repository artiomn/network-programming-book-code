#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <vector>

#if !defined(_WIN32)
extern "C"
{
#    include <netinet/tcp.h>
#    include <signal.h>
#    include <sys/ioctl.h>
}
#else
extern "C"
{
#    include <io.h>
}
#    include <cwctype>

#    define ioctl (ioctlsocket)

#endif

#include <fcntl.h>
#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>


const auto clients_count = 10;
const auto buffer_size = 4096;
using namespace std::literals;
namespace fs = std::filesystem;


#if !defined(MAX_PATH)
#    define MAX_PATH (256)
#endif

#if defined(_WIN32)
#    define close (_close)
#    define read (_read)
const wchar_t separ = fs::path::preferred_separator;
#else
const wchar_t separ = *reinterpret_cast<const wchar_t *>(&fs::path::preferred_separator);
#endif


void set_nonblock(SocketDescriptorType fd)
{
    const IoctlType flag = 1;

    if (ioctl(fd, FIONBIO, const_cast<IoctlType *>(&flag)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Setting non-blocking mode");
    }
}


class Transceiver
{
public:
    enum class IOStatus
    {
        ok,
        no_data,
        error,
        finished
    };


public:
    explicit Transceiver(socket_wrapper::Socket &&client_sock)
        : buffer_(buffer_size), client_sock_(std::move(client_sock))
    {
    }
    Transceiver(Transceiver &&t) : buffer_(buffer_size), client_sock_(std::move(t.client_sock_)) {}
    Transceiver(const Transceiver &) = delete;
    Transceiver() = delete;
    ~Transceiver()
    {
        if (file_descriptor_ != -1) close(file_descriptor_);
    }

public:
    const socket_wrapper::Socket &ts_socket() const { return client_sock_; }
    int file_descriptor() const { return file_descriptor_; }
    bool read_finished() const { return read_finished_; }
    size_t sent_size() const { return sent_size_; }

public:
    IOStatus send_buffer()
    {
        assert(true == static_cast<bool>(client_sock_));
        if (send_index_ == read_index_) return read_finished() ? IOStatus::finished : IOStatus::no_data;

        auto data_size = ((send_index_ > read_index_) ? buffer_.size() : read_index_) - send_index_;

        while (true)
        {
            auto result = send(client_sock_, &buffer_[send_index_], data_size, 0);
            std::cout << data_size << " bytes requested, " << result << " bytes was sent" << std::endl;

            if (-1 == result)
            {
                if (need_to_repeat()) continue;
                return IOStatus::error;
            }

            if (!result) return IOStatus::finished;

            sent_size_ += result;
            send_index_ = (send_index_ + result) % buffer_.size();

            break;
        }

        return IOStatus::ok;
    }

    IOStatus read_data()
    {
        if (read_finished_) return IOStatus::finished;
        if (-1 == file_descriptor_) return IOStatus::error;

        auto data_size = ((send_index_ <= read_index_) ? buffer_.size() - !send_index_ : send_index_ - 1) - read_index_;

        // Buffer is full.
        if (!data_size) return IOStatus::no_data;

        while (true)
        {
            auto result = read(file_descriptor_, &buffer_[read_index_], data_size);
            std::cout << data_size << " bytes requested, " << result << " bytes was read" << std::endl;

            if (-1 == result)
            {
                if (need_to_repeat()) continue;
                return IOStatus::error;
            }

            if (0 == result)
            {
                read_finished_ = true;
                // close(file_descriptor_);
                // file_descriptor_ = -1;
                return IOStatus::finished;
            }

            read_index_ = (read_index_ + result) % buffer_.size();

            break;
        }

        return IOStatus::ok;
    }

    int open_file(fs::path const &file_path)
    {
        std::cout << "Opening file " << file_path << "..." << std::endl;
#if !defined(_WIN32)
        file_descriptor_ = open(file_path.string().c_str(), O_NONBLOCK, O_RDONLY);
#else
        file_descriptor_ = _open(file_path.string().c_str(), _O_BINARY, _O_RDONLY);
#endif

        return file_descriptor_;
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
            recv_bytes += result % buffer_.size();
            if (size == recv_bytes) break;
        }

        buffer[recv_bytes] = '\0';

        auto result = std::string(buffer.begin(), buffer.begin() + recv_bytes);
        std::cout << "Request = \"" << result << "\"" << std::endl;

        return result;
    }

private:
    static bool need_to_repeat()
    {
        switch (errno)
        {
            case EINTR:
            case EAGAIN:
                // case EWOULDBLOCK: // EWOULDBLOCK == EINTR.
                return true;
        }

        return false;
    };

private:
    std::vector<char> buffer_;
    size_t read_index_ = 0;
    size_t send_index_ = 0;
    int file_descriptor_ = -1;
    socket_wrapper::Socket client_sock_;
    bool read_finished_ = false;
    size_t sent_size_ = 0;
};


class Client
{
public:
    explicit Client(socket_wrapper::Socket &&sock) : tsr_(std::move(sock))
    {
        std::cout << "Client [" << static_cast<int>(tsr_.ts_socket()) << "] "
                  << "was created..." << std::endl;
    }

    Client(Client &&client) : tsr_(std::move(client.tsr_)) {}

    ~Client()
    {
        std::cout << "Client [" << static_cast<int>(tsr_.ts_socket()) << "] "
                  << "removing..." << std::endl;
    }

public:
    operator SocketDescriptorType() const { return static_cast<SocketDescriptorType>(tsr_.ts_socket()); }
    int get_fd() const { return tsr_.file_descriptor(); }
    bool file_opened() const { return get_fd() != -1; }
    const fs::path &get_file_path() const { return file_path_; }
    bool read_finished() const { return tsr_.read_finished(); }
    size_t sent_size() const { return tsr_.sent_size(); }

public:
    int update_file_path()
    {
        auto fp = recv_file_path();

        if (file_opened() || (std::nullopt == fp)) return -1;
        file_path_ = *fp;

        return open_file(file_path_);
    }

    Transceiver::IOStatus read_data() { return tsr_.read_data(); }

    Transceiver::IOStatus send_data() { return tsr_.send_buffer(); }

private:
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

    int open_file(const fs::path &file_path)
    {
        if (!(fs::exists(file_path) && fs::is_regular_file(file_path))) return -1;

        return tsr_.open_file(file_path);
    }

private:
    Transceiver tsr_;
    fs::path file_path_;
};


template <size_t max_clients_count = clients_count>
class SelectProcessor
{
public:
    const size_t max_clients = max_clients_count;
    const std::string max_clients_msg = "Maximum clients limit exceeds!";

public:
    explicit SelectProcessor(socket_wrapper::Socket &server) : server_socket_(server), max_available_descriptor_(server)
    {
    }

private:
    void build_sock_list()
    {
        // Make empty set.
        FD_ZERO(&read_descriptors_set_);
        FD_ZERO(&write_descriptors_set_);
        FD_ZERO(&err_descriptors_set_);

        // If the client tries to connect() to the server socket, select() will treat this socket as "readable".
        // Then program must to accept a new connection.
        FD_SET(static_cast<SocketDescriptorType>(server_socket_), &read_descriptors_set_);

        for (auto &c : clients_)
        {
            if (!c.read_finished())
            {
                FD_SET(static_cast<SocketDescriptorType>(c), &read_descriptors_set_);
            }
            FD_SET(static_cast<SocketDescriptorType>(c), &write_descriptors_set_);
            FD_SET(static_cast<SocketDescriptorType>(c), &err_descriptors_set_);

#if !defined(WIN32)
            // Windows select() doesn't support file descriptors: it works only with sockets.

            if (c.file_opened() && !c.read_finished())
            {
                auto fd = c.get_fd();
                FD_SET(static_cast<int>(fd), &read_descriptors_set_);
                FD_SET(static_cast<int>(fd), &err_descriptors_set_);
                if (fd > max_available_descriptor_) max_available_descriptor_ = fd;
            }
#endif
            if (c > max_available_descriptor_) max_available_descriptor_ = c;
        }
    }

    void process_new_client()
    {
        auto client_sock = socket_wrapper::accept_client(server_socket_);

        if (clients_.size() >= max_clients)
        {
            std::cout << "Server can't accept new client (max = " << max_clients << ")" << std::endl;
            send(client_sock, max_clients_msg.c_str(), max_clients_msg.size(), 0);
            return;
        }

        // Set non-blocking mode.
        set_nonblock(client_sock);

        std::cout << "Client " << clients_.size() << " added." << std::endl;
        clients_.emplace_back(Client(std::move(client_sock)));
    }

public:
    void run_step()
    {
        timeval timeout = {.tv_sec = 1, .tv_usec = 0};
        build_sock_list();

        auto active_descriptors = select(
            max_available_descriptor_ + 1, &read_descriptors_set_, &write_descriptors_set_, &err_descriptors_set_,
            &timeout);

        if (active_descriptors < 0)
        {
            throw std::system_error(errno, std::system_category(), "select");
        }

        if (0 == active_descriptors)
        {
            // Nothing to do.
            return;
        }

        std::cout << "\n" << active_descriptors << " descriptors active..." << std::endl;
        // Accept new connection, if it's possible.
        if (FD_ISSET(static_cast<SocketDescriptorType>(server_socket_), &read_descriptors_set_)) process_new_client();

        for (auto client_iter = clients_.begin(); client_iter != clients_.end();)
        {
            if (FD_ISSET(static_cast<SocketDescriptorType>(*client_iter), &err_descriptors_set_))
            {
                // Client socket error.
                // remove_descriptor(*client_iter);
                std::cerr << "Socket error: " << *client_iter << std::endl;
                clients_.erase(client_iter++);
                continue;
            }

            if (FD_ISSET(client_iter->get_fd(), &err_descriptors_set_))
            {
                // Client file error.
                std::cerr << "File error: " << client_iter->get_fd() << std::endl;
                clients_.erase(client_iter++);
                continue;
            }

            if (FD_ISSET(static_cast<SocketDescriptorType>(*client_iter), &read_descriptors_set_))
            {
                std::cout << "Socket ready for reading: " << *client_iter << std::endl;
                if (-1 == client_iter->update_file_path())
                {
                    std::cerr << "File [re]opening error: " << client_iter->get_fd() << std::endl;
                    clients_.erase(client_iter++);
                    continue;
                }
            }

            try
            {
                auto &client = *client_iter;
                /*if (!client.file_opened())
                {
                    ++client_iter;
                    continue;
                }*/

                if (FD_ISSET(static_cast<SocketDescriptorType>(client), &write_descriptors_set_))
                {
                    std::cout << "Socket ready for writing: " << client << std::endl;
                    switch (client.send_data())
                    {
                        case Transceiver::IOStatus::error:
                        {
                            std::cerr << "Data sending error: " << *client_iter << std::endl;
                            clients_.erase(client_iter++);
                            continue;
                        }
                        case Transceiver::IOStatus::finished:
                        {
                            if (client.read_finished())
                            {
                                std::cout << "File " << client.get_file_path()
                                          << " was sent [size = " << client.sent_size() << " bytes]" << std::endl;
                                // Client disconnected: all file was sent.
                                clients_.erase(client_iter++);
                                continue;
                            }
                        }
                    }
                }

                if (FD_ISSET(client.get_fd(), &read_descriptors_set_))
                {
                    std::cout << "File ready for reading: " << client.get_fd() << std::endl;
                    if (Transceiver::IOStatus::error == client.read_data())
                    {
                        std::cerr << "File reading error: " << *client_iter << std::endl;
                        clients_.erase(client_iter++);
                        continue;
                    }
                }

                ++client_iter;
                continue;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
                clients_.erase(client_iter++);
            }
            catch (...)
            {
                std::cerr << "Unknown client error!" << std::endl;
                clients_.erase(client_iter++);
            }
        }
    }

private:
    socket_wrapper::Socket &server_socket_;
    std::list<Client> clients_;
    fd_set read_descriptors_set_;
    fd_set write_descriptors_set_;
    fd_set err_descriptors_set_;
    int max_available_descriptor_;
};


int main(int argc, const char *const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

#if !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = socket_wrapper::get_serv_info(argv[1]);
        if (!servinfo)
        {
            std::cerr << "Can't get servinfo!" << std::endl;
            return EXIT_FAILURE;
        }

        socket_wrapper::Socket server_sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (!server_sock)
        {
            throw std::system_error(errno, std::system_category(), "server socket");
        }

        set_reuse_addr(server_sock);

        if (bind(server_sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        if (listen(server_sock, clients_count) < 0)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }

        std::cout << "Listening on port " << argv[1] << "...\n"
                  << "Server path: " << fs::current_path() << std::endl;

        SelectProcessor sp(server_sock);

        while (true)
        {
            sp.run_step();
        }

        std::cout << "\n\nExiting normally\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << ": " << sock_wrap.get_last_error_string() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
