#include <iostream>

#include <libp2p/libp2p.hpp>
#include <libp2p/mplex.hpp>
#include <libp2p/noise.hpp>
#include <libp2p/tcp.hpp>


int main()
{
    auto transport = std::make_shared<libp2p::tcp::Transport>();
    auto security = std::make_shared<libp2p::noise::Crypto>();
    auto muxer = std::make_shared<libp2p::mplex::Multiplexer>();

    auto node = libp2p::NodeBuilder().setTransport(transport).setSecurity(security).setMuxer(muxer).build();

    node.start();
    std::cout << "Node is running!" << std::endl;

    return EXIT_SUCCESS;
}
