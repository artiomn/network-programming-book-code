extern "C"
{
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ares.h>
}

#include <iostream>


void dns_callback (void* arg, int status, int timeouts, struct hostent* host)
{
    (void)arg;

    if (ARES_SUCCESS == status)
        std::cout << host->h_name << std::endl;
    else
        std::cerr << "lookup failed: " << status << std::endl;
}


void main_loop(ares_channel &channel)
{
    int nfds, count;
    fd_set readers, writers;
    timeval tv, *tvp;

    while (true)
    {
        FD_ZERO(&readers);
        FD_ZERO(&writers);
        nfds = ares_fds(channel, &readers, &writers);

        if (!nfds) break;

        tvp = ares_timeout(channel, nullptr, &tv);
        count = select(nfds, &readers, &writers, nullptr, tvp);

        if (count < 0) break;

        ares_process(channel, &readers, &writers);
     }

}


int main(int argc, const char *argv[])
{
    struct in_addr ip;
    int res;

    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " ip.address" << std::endl;
        return EXIT_FAILURE;
    }

    inet_aton(argv[1], &ip);
    ares_channel channel;

    if ((res = ares_init(&channel)) != ARES_SUCCESS)
    {
        std::cerr << "ares failed: " << res << std::endl;
        return EXIT_FAILURE;
    }

    ares_gethostbyaddr(channel, &ip, sizeof ip, AF_INET, dns_callback, nullptr);
    main_loop(channel);

    return EXIT_SUCCESS;
}

