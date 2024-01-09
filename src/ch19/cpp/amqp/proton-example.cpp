#include <iostream>

#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/tracker.hpp>


class hello_world : public proton::messaging_handler
{
    std::string conn_url_;
    std::string addr_;

public:
    hello_world(const std::string& u, const std::string& a) : conn_url_(u), addr_(a) {}

    void on_container_start(proton::container& c) override { c.connect(conn_url_); }

    void on_connection_open(proton::connection& c) override
    {
        c.open_receiver(addr_);
        c.open_sender(addr_);
    }

    void on_sendable(proton::sender& s) override
    {
        proton::message m("Hello World!");
        s.send(m);
        s.close();
    }

    void on_message(proton::delivery& d, proton::message& m) override
    {
        std::cout << m.body() << std::endl;
        d.connection().close();
    }
};


int main(int argc, char** argv)
{
    try
    {
        std::string conn_url = argc > 1 ? argv[1] : "//127.0.0.1:5672";
        std::string addr = argc > 2 ? argv[2] : "examples";

        hello_world hw(conn_url, addr);
        proton::container(hw).run();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_FAILURE;
}
