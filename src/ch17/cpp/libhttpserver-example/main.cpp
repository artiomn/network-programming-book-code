#include <httpserver.hpp>


using namespace httpserver;

class hello_world_resource : public http_resource
{
public:
    std::shared_ptr<http_response> render(const http_request&)
    {
        return std::make_shared<http_response>(string_response("Hello, World!"));
    }
};


int main(int argc, const char* const argv[])
{
    webserver ws = create_webserver(12345);

    hello_world_resource hwr;
    ws.register_resource("/hello", &hwr);
    ws.start(true);

    return EXIT_SUCCESS;
}
