#include <iomanip>
#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"


int main(int argc, const char* const argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <http_server> <https_server>" << std::endl;
        return EXIT_FAILURE;
    }
    // HTTP
    httplib::Client cli(argv[1]);

    // HTTPS
    httplib::SSLClient ssl_cli(argv[2]);
    ssl_cli.set_ca_cert_path("/etc/ssl/certs/ca-certificates.crt");

    auto res = cli.Get("/");
    std::cout
        << res->status << "\n"
        << res->body
        << std::endl;

    res = ssl_cli.Get("/");

    std::cout
        << res->status << "\n"
        << res->body
        << std::endl;
}

