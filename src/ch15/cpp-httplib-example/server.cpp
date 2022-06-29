#include <iostream>
#include <string>
#include "httplib.h"


int main(int argc, const char* const argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    // HTTP
    httplib::Server svr;
    // HTTPS
    // httplib::SSLServer ssl_svr("./cpp-httplib-cert.pem", "./cpp-httplib-key.pem");

    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
    {
        res.set_content("Hello World!", "text/plain");
    });

    std::cout << "Server started..." << std::endl;
    svr.listen("0.0.0.0", std::stoi(argv[1]));
}

