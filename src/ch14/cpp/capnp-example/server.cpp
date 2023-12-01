#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <kj/debug.h>

#include <iostream>
#include <memory>
#include <string>

// cppcheck-suppress missingInclude
#include "echo.capnp.h"


class ExampleEchoServiceImpl final : public Echo::Server
{
public:
    ExampleEchoServiceImpl() {}

    kj::Promise<void> echo(EchoContext context)
    {
        auto echo_value = context.getParams().getEcho_request();

        std::cout << "Client request: " << std::string(echo_value) << std::endl;
        context.getResults().setEcho_reply("from server = " + std::string(echo_value));
        return kj::READY_NOW;
    }
};


int main()
{
    // Set up a server.
    capnp::EzRpcServer server(kj::heap<ExampleEchoServiceImpl>(), "0.0.0.0:12345");

    // Write the port number to stdout, in case it was chosen automatically.
    auto& waitScope = server.getWaitScope();
    uint port = server.getPort().wait(waitScope);
    if (!port)
    {
        // The address format "unix:/path/to/socket" opens a unix domain socket,
        // in which case the port will be zero.
        std::cout << "Listening on Unix socket..." << std::endl;
    }
    else
    {
        std::cout << "Listening on port " << port << "..." << std::endl;
    }

    // Run forever, accepting connections and handling requests.
    kj::NEVER_DONE.wait(waitScope);
    return EXIT_SUCCESS;
}
