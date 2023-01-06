#include <pistache/endpoint.h>


using namespace Pistache;

class HelloHandler : public Http::Handler
{
public:

    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request& request, Http::ResponseWriter response) override
    {
        response.send(Http::Code::Ok, "Hello, World\n");
    }
};


int main(int argc, const char * const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    Http::listenAndServe<HelloHandler>(Address(Ipv4::any(), Port(std::stoi(argv[1]))));
}
