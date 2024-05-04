#include <Poco/Net/DNS.h>

#include <iostream>


using Poco::Net::DNS;
using Poco::Net::HostEntry;
using Poco::Net::IPAddress;


int main(int argc, const char* argv[])
{
    const HostEntry& entry = DNS::hostByName("www.pocoproject.org");
    std::cout << "Canonical Name: " << entry.name() << std::endl;

    const HostEntry::AliasList& aliases = entry.aliases();

    for (auto const& alias : aliases) std::cout << "Alias: " << alias << std::endl;

    const HostEntry::AddressList& addrs = entry.addresses();
    for (auto const& addr : addrs) std::cout << "Address: " << addr.toString() << std::endl;

    return EXIT_SUCCESS;
}
