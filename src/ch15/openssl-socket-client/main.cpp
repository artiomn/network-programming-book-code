#include <iostream>
#include <string>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

extern "C"
{
#include <openssl/ssl.h>
#include <openssl/err.h>
}


const auto buffer_size = 256;


bool recv_packet(SSL *ssl)
{
    char buf[buffer_size];
    int len = 0;

    do
    {
        len = SSL_read(ssl, buf, buffer_size - 1);
        buf[len] = 0;
        std::cout << buf << std::endl;
    }
    while (len > 0);

    if (len < 0)
    {
        switch (SSL_get_error(ssl, len))
        {
            // Not an error.
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return true;
            break;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
                return false;
        }
    }

    return true;
}


bool send_packet(const std::string &buf, SSL *ssl)
{
    int len = SSL_write(ssl, buf.c_str(), buf.size());
    if (len < 0)
    {
        int err = SSL_get_error(ssl, len);
        switch (err)
        {
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
                return true;
            break;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
            default:
                return false;
        }
    }

    return false;
}


void log_ssl()
{
    for (int err = ERR_get_error(); err; err = ERR_get_error())
    {
        char *str = ERR_error_string(err, 0);
        if (!str) return;
        std::cerr << str << std::endl;
    }
}


int main(int argc, const char * const argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short port = std::stoi(argv[2]);

    addrinfo hints =
    {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    // Results.
    addrinfo *servinfo = nullptr;
    int status = 0;

    if ((status = getaddrinfo(argv[1], nullptr, &hints, &servinfo)) != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in sa = {*reinterpret_cast<const sockaddr_in* const>(servinfo->ai_addr)};
    sa.sin_port = htons(port);

    freeaddrinfo(servinfo);

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!sock)
    {
        std::cerr << "Error creating socket." << std::endl;
        return EXIT_FAILURE;
    }

    if (connect(sock, reinterpret_cast<const struct sockaddr *>(&sa), sizeof(sa)))
    {
        std::cerr << "Error connecting to server." << std::endl;
        return -1;
    }

    SSL *ssl = nullptr;

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD *meth = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ctx);

    if (!ssl)
    {
        std::cerr << "Error creating SSL." << std::endl;
        log_ssl();
        return EXIT_FAILURE;
    }

    SSL_set_fd(ssl, sock);

    int err = SSL_connect(ssl);
    if (err <= 0)
    {
        std::cerr << "Error creating SSL connection.  err = " << err << std::endl;
        log_ssl();
        return EXIT_FAILURE;
    }
    std::cout << "SSL connection using " << SSL_get_cipher(ssl) << std::endl;

    std::string request = {"GET / HTTP/1.1\r\n\r\n"};
    send_packet(request, ssl);
    recv_packet(ssl);

    SSL_shutdown(ssl);

    return EXIT_SUCCESS;
}

