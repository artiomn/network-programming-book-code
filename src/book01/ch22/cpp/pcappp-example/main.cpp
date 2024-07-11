#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/PcapFileDevice.h>

#include <iostream>


int main(int argc, char* argv[])
{
    pcpp::PcapFileReaderDevice reader("1_packet.pcap");

    if (!reader.open())
    {
        std::cerr << "Error opening the pcap file" << std::endl;
        return EXIT_FAILURE;
    }

    pcpp::RawPacket rawPacket;
    if (!reader.getNextPacket(rawPacket))
    {
        std::cerr << "Couldn't read the first packet in the file" << std::endl;
        return EXIT_FAILURE;
    }

    pcpp::Packet parsedPacket(&rawPacket);

    if (parsedPacket.isPacketOfType(pcpp::IPv4))
    {
        pcpp::IPv4Address srcIP = parsedPacket.getLayerOfType<pcpp::IPv4Layer>()->getSrcIPv4Address();
        pcpp::IPv4Address destIP = parsedPacket.getLayerOfType<pcpp::IPv4Layer>()->getDstIPv4Address();

        std::cout << "Source IP is '" << srcIP << "'; Dest IP is '" << destIP << "'" << std::endl;
    }

    reader.close();

    return EXIT_SUCCESS;
}
