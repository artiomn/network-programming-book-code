#include <server/http/https_client.h>
#include <string/string_utils.h>

#include <iostream>


int main(int argc, char** argv)
{
    std::string address = "127.0.0.1";
    if (argc > 1) address = argv[1];

    std::cout << "HTTPS server address: " << address << std::endl;

    std::cout << std::endl;

    auto service = std::make_shared<CppServer::Asio::Service>();

    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->load_verify_file("ca.pem");

    auto client = std::make_shared<CppServer::HTTP::HTTPSClientEx>(service, context, address, "https");

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    try
    {
        std::string line;
        while (getline(std::cin, line))
        {
            if (line.empty()) break;

            if (line == "!")
            {
                std::cout << "Client reconnecting...";
                client->ReconnectAsync();
                std::cout << "Done!" << std::endl;
                continue;
            }

            auto commands = CppCommon::StringUtils::Split(line, ' ', true);
            if (commands.size() < 2)
            {
                std::cout << "HTTP method and URL must be entered!" << std::endl;
                continue;
            }

            if (CppCommon::StringUtils::ToUpper(commands[0]) == "HEAD")
            {
                auto response = client->SendHeadRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "GET")
            {
                auto response = client->SendGetRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "POST")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPostRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "PUT")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPutRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "DELETE")
            {
                auto response = client->SendDeleteRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "OPTIONS")
            {
                auto response = client->SendOptionsRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "TRACE")
            {
                auto response = client->SendTraceRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else
                std::cout << "Unknown HTTP method: " << commands[0] << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return EXIT_SUCCESS;
}
