#include <iostream>
#include <string>
#include <cctype>
#include <cerrno>

#include <pcap.h>

#include "packet_printer.h"

// Default snap length (maximum bytes per packet to capture).
const auto SNAP_LEN = 1518;
const auto MAX_PACKET_TO_CAPTURE = 10;

static void pcap_callback(u_char *args, const pcap_pkthdr *header, const u_char *packet)
{
    static PacketPrinter p_printer;

    p_printer.got_packet(args, header, packet);
};


int main(int argc, const char * const argv[])
{
    std::string dev;
    // Pcap error buffer.
    char errbuf[PCAP_ERRBUF_SIZE];
    // Packet capture handle.
    pcap_t *handle;

    std::string filter_exp = "ip";
    // Compiled filter program (expression).
    bpf_program fp;
    bpf_u_int32 mask;
    bpf_u_int32 net;
    // Number of packets to capture.
    int num_packets = MAX_PACKET_TO_CAPTURE;

    // Check for capture device name on command-line.
    if (2 == argc)
    {
        dev = argv[1];
    }
    else if (argc > 2)
    {
        std::cerr << "error: unrecognized command-line options\n" << std::endl;
        std::cout
            << "Usage: " << argv[0] << " [interface]\n\n"
            << "Options:\n"
            << "    interface    Listen on <interface> for packets.\n"
            << std::endl;

        exit(EXIT_FAILURE);
    }
    else
    {
        // Find a capture device if not specified on command-line.
        pcap_if_t *all_devs_sp;

        if (pcap_findalldevs(&all_devs_sp, errbuf) != 0 || nullptr == all_devs_sp)
        {
            std::cerr << "Couldn't find default device: \"" << errbuf << "\"" << std::endl;
            return EXIT_FAILURE;
        }
        dev = all_devs_sp->name;
        pcap_freealldevs(all_devs_sp);
    }

    // Get network number and mask associated with capture device.
    if (-1 == pcap_lookupnet(dev.c_str(), &net, &mask, errbuf))
    {
        std::cerr << "Couldn't get netmask for device \"" << dev << "\": " << errbuf << std::endl;
        net = 0;
        mask = 0;
    }

    // Print capture info.
    std::cout
        << "Device: " << dev << "\n"
        << "Network mask: " << mask << "\n"
        << "Network: " << net << "\n"
        << "Number of packets: " << num_packets << "\n"
        << "Filter expression: " << filter_exp << std::endl;

    // Open capture device.
    handle = pcap_open_live(dev.c_str(), SNAP_LEN, 1, 1000, errbuf);
    if (nullptr == handle)
    {
        std::cerr << "Couldn't open device \"" << dev << "\": " << errbuf << "!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Make sure we're capturing on an Ethernet device.
    if (pcap_datalink(handle) != DLT_EN10MB)
    {
        std::cerr << "\"" << dev << "\" is not an Ethernet!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Compile the filter expression.
    if (pcap_compile(handle, &fp, filter_exp.c_str(), 0, net) == -1)
    {
        std::cerr << "Couldn't parse filter \"" << filter_exp << "\": " << pcap_geterr(handle) << "!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Apply the compiled filter.
    if (-1 == pcap_setfilter(handle, &fp))
    {
        std::cerr << "Couldn't install filter \"" << filter_exp << "\": " << pcap_geterr(handle) << "!" << std::endl;
        exit(EXIT_FAILURE);
    }

    pcap_loop(handle, num_packets, pcap_callback, nullptr);

    // Cleanup.
    pcap_freecode(&fp);
    pcap_close(handle);

    std::cout << "\nCapture complete." << std::endl;

    return EXIT_SUCCESS;
}
