// Simple libev++-based echo server.

extern "C"
{
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
}

#include <cerrno>
#include <iostream>
#include <list>
#include <tuple>
#include <vector>

#include <ev++.h>


typedef std::vector<char> Buffer;

class EchoInstance
{
public:
    EchoInstance(int s) : working_(true), sfd_(s)
    {
        fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
        std::cout << "Got connection..." << std::endl;
        ++total_clients_;

        io_.set<EchoInstance, &EchoInstance::callback>(this);

        io_.start(s, ev::READ);
    }

    bool is_working() const
    {
        return working_;
    }

private:
    // Generic callback
    void callback(ev::io &watcher, int revents)
    {
        if (EV_ERROR & revents)
        {
            std::cerr << "Got invalid event!" << std::endl;
            return;
        }

        if (revents & EV_READ) read_cb(watcher);
        if (revents & EV_WRITE) write_cb(watcher);
        if (write_queue.empty())
	{
            io_.set(ev::READ);
        }
	else
	{
            io_.set(ev::READ|ev::WRITE);
        }
    }

    // Socket is writable
    void write_cb(ev::io &watcher)
    {
        if (write_queue.empty())
	{
            io_.set(ev::READ);
            return;
        }

        auto &btuple = write_queue.front();
        auto &buffer = std::get<0>(btuple);
        auto buffer_pos = std::get<1>(btuple);

        ssize_t written = write(watcher.fd, buffer.data() + buffer_pos,
								buffer.size() - buffer_pos);
        if (written < 0)
        {
            std::cerr << "Read error!" << std::endl;
            return;
        }

        buffer_pos += written;
        std::get<1>(btuple) = buffer_pos;
        if (buffer.size() <= buffer_pos)
        {
            write_queue.pop_front();
        }
    }

    // Receive message from client socket
    void read_cb(ev::io &watcher)
    {
        Buffer buffer;
        buffer.resize(1024);

        ssize_t nread = recv(watcher.fd, buffer.data(), buffer.size(), 0);

        if (nread < 0)
		{
            std::cerr << "Read error!" << std::endl;
            return;
        }

		if (nread)
		{
            // Send message bach to the client.
			buffer.resize(nread);
            write_queue.push_back(std::make_tuple(std::move(buffer), 0));
        }
		else
		{
			working_ = false;
		}
    }

    virtual ~EchoInstance()
	{
        // Stop and free watcher if client socket is closing
        io_.stop();
        close(sfd_);
        std::cout << --total_clients_ << " client(s) connected.\n" << std::endl;
    }

private:
    bool working_;
    ev::io io_;
    static int total_clients_;
    int sfd_;

    // Buffers that are pending write
    std::list<std::tuple<Buffer, size_t>> write_queue;
};


class EchoServer
{
private:
    ev::io io_;
    ev::sig sio_;
    int s_;

public:
    void io_accept(ev::io &watcher, int revents)
    {
        if (EV_ERROR & revents)
        {
            std::cerr << "Got invalid event" << std::endl;
            return;
        }

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_sd = accept(watcher.fd,
            reinterpret_cast<sockaddr *>(&client_addr), &client_len);

        if (client_sd < 0)
        {
            std::cerr << "Accept error" << std::endl;
            return;
        }

        EchoInstance *client = new EchoInstance(client_sd);
    }

    static void signal_cb(ev::sig &signal, int revents)
    {
        (void)revents;

        signal.loop.break_loop();
    }

    EchoServer(int port)
    {
        std::cout << "Listening on port " << port << std::endl;

        struct sockaddr_in addr;

        s_ = socket(PF_INET, SOCK_STREAM, 0);

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(s_, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            std::cerr << "Bind error!" << std::endl;
        }

        fcntl(s_, F_SETFL, fcntl(s_, F_GETFL, 0) | O_NONBLOCK);

        listen(s_, 5);

        io_.set<EchoServer, &EchoServer::io_accept>(this);
        io_.start(s_, ev::READ);

        sio_.set<&EchoServer::signal_cb>();
        sio_.start(SIGINT);
    }

    ~EchoServer()
    {
        shutdown(s_, SHUT_RDWR);
        close(s_);
    }
};


int EchoInstance::total_clients_ = 0;

int main(int argc, char **argv)
{
    int port = 8192;

    if (argc > 1) port = atoi(argv[1]);

    ev::default_loop       loop;
    EchoServer           echo(port);

    loop.run(0);

    return EXIT_SUCCESS;
}
