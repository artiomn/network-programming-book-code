#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SocketAddress.h>
#include <iostream>
#include <iomanip>


int main(int argc, const char* argv[])
{
    Poco::Net::ServerSocket srv(8080);
    for (;;)
    {
        Poco::Net::StreamSocket ss = srv.acceptConnection();
        Poco::Net::SocketStream str(ss);
        str << "HTTP/1.0 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "\r\n"
               "<html><head><title>My 1st Web Server</title></head>"
               "<body><h1>Hello, world!</h1></body></html>"
               << std::flush;
    }

    return EXIT_SUCCESS;
}

