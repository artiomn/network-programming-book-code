#include <uWebSockets/App.h>

#include <iostream>


int main()
{
    uWS::SSLApp({.key_file_name = "key.pem", .cert_file_name = "cert.pem", .passphrase = "1234"})
        .get("/*", [](auto *res, auto *req) { res->end("Hello world!"); })
        .listen(
            3000,
            [](auto *listen_socket)
            {
                if (listen_socket)
                {
                    std::cout << "Listening on port " << 3000 << std::endl;
                }
            })
        .run();

    std::cout << "Failed to listen on port 3000" << std::endl;
}
