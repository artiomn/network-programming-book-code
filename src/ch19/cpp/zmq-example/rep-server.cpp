#include <algorithm>
#include <iostream>
#include <string>

#include <zmq.hpp>

#ifndef _WIN32
#    include <unistd.h>
#else
#    include <windows.h>
#    define sleep(n) Sleep(n)
#endif


int main()
{
    zmq::context_t context(2);
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:12345");

    const std::string msg_string = "Pong";

    while (true)
    {
        zmq::message_t request;

        if (!socket.recv(request, zmq::recv_flags::none)) throw std::logic_error("recv");
        std::cout << "Received Ping" << std::endl;

        sleep(1);

        zmq::message_t reply(msg_string.size());
        std::copy(msg_string.begin(), msg_string.end(), static_cast<char*>(reply.data()));
        if (!socket.send(reply, zmq::send_flags::none)) throw std::logic_error("send");
    }
    return EXIT_SUCCESS;
}
