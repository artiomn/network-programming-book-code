#include <iostream>
#include <memory>
#include <vector>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/PhoneService.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


int main()
{
    std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    PhoneServiceClient client(protocol);

    try
    {
        transport->open();

        std::vector<Phone> numbers;
        client.findAll(numbers);

        std::cout << "Numbers: " << std::endl;
        for (const auto &n : numbers) std::cout << n << "\n";
        std::cout << std::endl;
        transport->close();
    }
    catch (const TException& tx)
    {
        std::cout << "ERROR: " << tx.what() << std::endl;
    }
}
