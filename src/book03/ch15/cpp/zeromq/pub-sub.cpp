#include <array>
#include <iostream>
#include <string>

#include <zmq.hpp>


using namespace std::chrono_literals;


int main()
{
    std::string ra{"inproc://route_"};
    std::string pa{"inproc://publish_"};

    zmq::context_t context;

    zmq::socket_t publish_socket_{context, zmq::socket_type::xpub};
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};

    publish_socket_.bind(pa);

    sub_socket.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
    sub_socket.connect(pa);

    while (true)
    {
        zmq::message_t message{std::string{"test"}};

        std::cout << "Sending msg" << std::endl;
        zmq::send_result_t send_result;
        do
        {
            send_result = publish_socket_.send(message, zmq::send_flags::none);
        } while (!send_result.has_value());

        std::cout << "Receiving msg" << std::endl;

        zmq::message_t msg;
        zmq::recv_result_t result;
        std::array<zmq_pollitem_t, 1> items = {zmq_pollitem_t{.socket = sub_socket.handle(), .events = ZMQ_POLLIN}};

        std::cout << "Running poll()" << std::endl;

        if (zmq::poll<1>(items, 3000ms))
        {
            std::cout << "Poll() successful, receiving data" << std::endl;
            do
            {
                result = sub_socket.recv(msg, zmq::recv_flags::dontwait);

                if (result.has_value())
                    std::cout << "Endpoint received " << result.value() << std::endl;
                else
                    std::cout << "Endpoint receiving error [EAGAIN]!" << std::endl;
            } while (!result.has_value());
        }
        else
        {
            std::cout << "Poll() returned 0, exiting" << std::endl;
        }
    }
}
