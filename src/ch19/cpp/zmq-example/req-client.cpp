#include <iostream>
#include <string>

#include <zmq.hpp>


int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::req);

    std::cout << "Connecting to hello world server..." << std::endl;
    socket.connect("tcp://localhost:12345");

    const std::string msg_string = "Ping";

    for (int request_nbr = 0; request_nbr != 10; ++request_nbr)
    {
        zmq::const_buffer request = zmq::buffer(msg_string);

        std::cout << "Sending " << msg_string << " " << request_nbr << "..." << std::endl;
        if (!socket.send(request, zmq::send_flags::none)) throw std::logic_error("send");

        zmq::message_t reply;
        if (!socket.recv(reply, zmq::recv_flags::none)) throw std::logic_error("recv");
        std::cout << "Received " << request_nbr << std::endl;
    }
    return EXIT_SUCCESS;
}
