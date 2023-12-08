#include <uWebSockets/App.h>

#include <iostream>


struct UserData
{
};


int main()
{
    uWS::SSLApp({.key_file_name = "key.pem", .cert_file_name = "cert.pem", .passphrase = "1234"})
        .get("/", [](auto* res, auto* req) { res->end("Hello world!"); })
        .ws<UserData>(
            "/ws/", {.compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
                     .maxPayloadLength = 16 * 1024 * 1024,
                     .idleTimeout = 600,
                     .maxBackpressure = 16 * 1024 * 1024,
                     .closeOnBackpressureLimit = false,
                     .resetIdleTimeoutOnSend = true,
                     .sendPingsAutomatically = true,
                     .upgrade = nullptr,
                     .open =
                         [](auto* ws)
                     {
                         // UserData* data = static_cast<UserData*>(ws->getUserData());
                         std::cout << "Connection" << std::endl;
                     },
                     .dropped =
                         [](auto* /*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/)
                     {
                         /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
                     },
                     .message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {},
                     .drain = [](auto* ws) {},
                     .ping = [](auto* ws, std::string_view pingmes) {},
                     .pong = [](auto* ws, std::string_view pongmes) {},
                     .close =
                         [](auto* ws, int code, std::string_view message)
                     {
                         // auto* data = static_cast<UserData*>(ws->getUserData());
                         // std::cout << "User \"" << data->user_name << "\" has disconnected!" << std::endl;
                     }})
        .listen(
            3000,
            [](auto* listen_socket)
            {
                if (listen_socket)
                {
                    std::cout << "Listening on port " << 3000 << std::endl;
                }
            })
        .run();

    std::cout << "Failed to listen on port 3000" << std::endl;
}
