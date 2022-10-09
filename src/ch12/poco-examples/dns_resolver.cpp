#include <iostream>
#include <Poco/Net/DNS.h>


using Poco::Net::DNS;
using Poco::Net::IPAddress;
using Poco::Net::HostEntry;


int main(int argc, const char* argv[])
{
    const HostEntry& entry = DNS::hostByName("www.pocoproject.org");
    std::cout << "Canonical Name: " << entry.name() << std::endl;

    const HostEntry::AliasList& aliases = entry.aliases();

    for (auto const &alias : aliases)
        std::cout << "Alias: " << alias << std::endl;

    const HostEntry::AddressList& addrs = entry.addresses();
    for (auto const &addr : addrs)
        std::cout << "Address: " << addr.toString() << std::endl;

    return EXIT_SUCCESS;
}

