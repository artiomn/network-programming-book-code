#include <iostream>
#include <memory>
#include <cstdlib>
#include <restbed>


using namespace std;
using namespace restbed;


void post_method_handler(const shared_ptr<Session> session)
{
    const auto request = session->get_request();

    std::cout << "Handler was activated..." << std::endl;
    int content_length = request->get_header("Content-Length", 0);

    session->fetch(content_length, [](const shared_ptr<Session> session, const Bytes& body)
    {
        std::cout
            << body.size() << "\n"
            << body.data()
            << std::endl;

        const std::string message { "Framework testing successful...\n" };
        session->close(OK, message.c_str(), { { "Content-Length", std::to_string(message.size()) } });
    });
}


int main(int argc, const char* const argv[])
{
    int port = (argc > 1) ? atoi(argv[1]) : 1984;

    auto resource = make_shared<Resource>();
    const std::string path = "/";
    const std::string method = "POST";

    resource->set_path(path);
    resource->set_method_handler(method, post_method_handler);

    auto settings = make_shared<Settings>();
    settings->set_port(port);
    settings->set_default_header("Connection", "close");

    Service service;
    service.publish(resource);
    std::cout
        << "Service will be started on the port "
        << port << "...\n"
        << "Use: " << method << " " << path
        << std::endl;
    service.start(settings);

    return EXIT_SUCCESS;
}

