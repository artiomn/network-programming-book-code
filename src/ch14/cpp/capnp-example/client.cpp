#include <capnp/ez-rpc.h>
#include <kj/debug.h>

#include <iostream>
#include <string>

// cppcheck-suppress missingInclude
#include "echo.capnp.h"


int main()
{
    capnp::EzRpcClient client("localhost:12345");
    Echo::Client echo_client = client.getMain<Echo>();
    auto request = echo_client.echoRequest();
    request.setEcho_request("This is echo message");

    std::cout << "Sending client request: \"" << std::string(request.getEcho_request().cStr()) << "\"" << std::endl;
    auto result_promise = request.send();

    auto& wait_scope = client.getWaitScope();
    auto response = result_promise.wait(wait_scope);
    std::cout << "Response received: " << std::string(response.getEcho()) << std::endl;

    return EXIT_SUCCESS;
}
