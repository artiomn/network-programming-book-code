// Simple libevent-based echo server.

extern "C"
{
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>
}

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>


void echo_read_cb(bufferevent *bev, void *ctx)
{
    // This callback is invoked when there is data to read on bev.
    evbuffer *input = bufferevent_get_input(bev);
    evbuffer *output = bufferevent_get_output(bev);

    // Copy all the data from the input buffer to the output buffer.
    evbuffer_add_buffer(output, input);
}


void echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR) std::cerr << "Error from bufferevent!" << std::endl;
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
    {
        bufferevent_free(bev);
    }
}


void accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *ctx)
{
    // Установить bufferevent для нового соединения.
    event_base *base = evconnlistener_get_base(listener);
    bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, echo_read_cb, nullptr, echo_event_cb, nullptr);

    bufferevent_enable(bev, EV_READ|EV_WRITE);
}


void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    std::cerr
			<< "Got an error " << err << evutil_socket_error_to_string(err)
			<< " on the listener.\n"
			<< "Shutting down."
			<< std::endl;

    event_base_loopexit(base, nullptr);
}


int main(int argc, const char *argv[])
{
    event_base *base;
    evconnlistener *listener;
    sockaddr_in sin = {0};

    int port = 9876;

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port!" << std::endl;
        return EXIT_FAILURE;
    }

    base = event_base_new();

    if (!base)
    {
        std::cerr << "Couldn't open event base!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listening on port " << port << std::endl;
    // This is an INET address.
    sin.sin_family = AF_INET;
    // Listen on 0.0.0.0
    sin.sin_addr.s_addr = htonl(0);
    // Listen on the given port.
    sin.sin_port = htons(port);

    listener = evconnlistener_new_bind(base, accept_conn_cb, nullptr,
                                       LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                       reinterpret_cast<const sockaddr*>(&sin), sizeof(sin));

    if (!listener)
    {
        std::cerr << "Couldn't create listener" << std::endl;
        return EXIT_FAILURE;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);

    event_base_dispatch(base);
    return EXIT_SUCCESS;
}
