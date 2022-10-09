#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/StreamCopier.h>
#include <iostream>


int main(int argc, const char* argv[])
{
    Poco::Net::SocketAddress sa("www.appinf.com", 80);
    Poco::Net::StreamSocket socket(sa);
    Poco::Net::SocketStream s_stream(socket);

    s_stream << "GET / HTTP/1.1\r\n"
                "Host: www.google.com\r\n"
                "\r\n";
    s_stream.flush();
    Poco::StreamCopier::copyStream(s_stream, std::cout);

    return EXIT_SUCCESS;
}

