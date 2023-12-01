extern "C"
{
#include <openssl/err.h>
#include <openssl/ssl.h>
}

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <iostream>
#include <memory>
#include <string>


using ContextPtr = std::unique_ptr<SSL_CTX, decltype(SSL_CTX_free) &>;
const uint16_t port = 4433;


socket_wrapper::Socket create_socket(int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    auto s = socket_wrapper::Socket(AF_INET, SOCK_STREAM, 0);

    if (!s)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0)
    {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}


ContextPtr create_context()
{
    const SSL_METHOD *method = TLS_server_method();
    ContextPtr ctx(SSL_CTX_new(method), SSL_CTX_free);

    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}


void configure_context(ContextPtr &ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx.get(), "server.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx.get(), "key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char **argv)
{
    socket_wrapper::SocketWrapper sock_wrap;
    auto ctx = std::move(create_context());

    configure_context(ctx);

    auto sock = std::move(create_socket(port));
    std::cout << "Serve on port " << port << std::endl;

    // Handle connections.
    while (true)
    {
        SSL *ssl;
        const std::string reply{"test\n"};

        socket_wrapper::Socket client{socket_wrapper::accept_client(sock)};

        std::cout << "Accepted client..." << std::endl;
        ssl = SSL_new(ctx.get());
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0)
        {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            std::cout << "Sending text \"" << reply << "\"" << std::endl;
            SSL_write(ssl, reply.c_str(), reply.size());
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
}
