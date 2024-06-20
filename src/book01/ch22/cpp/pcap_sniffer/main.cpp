extern "C"
{
#include <pcap.h>
}

#include <array>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <string>

#include "packet_printer.h"

// Default snap length (maximum bytes per packet to capture).
constexpr auto SNAP_LEN = 1518;
constexpr auto MAX_PACKET_TO_CAPTURE = 10;

static void pcap_callback(u_char *args, const pcap_pkthdr *header, const u_char *packet)
{
    static PacketPrinter p_printer;

    p_printer.got_packet(args, header, packet);
};


int main(int argc, const char *const argv[])
{
    std::string dev;
    // Pcap error buffer.
    std::array<char, PCAP_ERRBUF_SIZE> errbuf;

    // Check for capture device name on command-line.
    if (2 == argc)
    {
        assert(argv[1]);
        dev = argv[1];
    }
    else if (argc > 2)
    {
        std::cerr << "error: unrecognized command-line options\n" << std::endl;
        std::cout << "Usage: " << argv[0] << " [interface]\n\n"
                  << "Options:\n"
                  << "    interface    Listen on <interface> for packets.\n"
                  << std::endl;

        return EXIT_FAILURE;
    }
    else
    {
        // Find a capture device if not specified on command-line.
        pcap_if_t *all_devs_sp = nullptr;

        if (pcap_findalldevs(&all_devs_sp, errbuf.data()) != 0 || nullptr == all_devs_sp)
        {
            std::cerr << "Couldn't find default device: \"" << errbuf.data() << "\"" << std::endl;
            return EXIT_FAILURE;
        }
        assert(all_devs_sp->name);
        dev = all_devs_sp->name;
        pcap_freealldevs(all_devs_sp);
    }

    bpf_u_int32 mask;
    bpf_u_int32 net;

    // Get network number and mask associated with capture device.
    if (-1 == pcap_lookupnet(dev.c_str(), &net, &mask, errbuf.data()))
    {
        std::cerr << "Couldn't get netmask for device \"" << dev << "\": " << errbuf.data() << std::endl;
        net = mask = 0;
    }

    // Number of packets to capture.
    constexpr int num_packets = MAX_PACKET_TO_CAPTURE;
    constexpr char filter_exp[] = "ip or ip6";

    // Print capture info.
    std::cout << "Device: " << dev << "\n"
              << "Network mask: " << mask << "\n"
              << "Network: " << net << "\n"
              << "Number of packets: " << num_packets << "\n"
              << "Filter expression: " << filter_exp << std::endl;

    // Open capture device and get packet capture handle.
    pcap_t *const handle = pcap_open_live(dev.c_str(), SNAP_LEN, 1, 1000, errbuf.data());
    if (nullptr == handle)
    {
        std::cerr << "Couldn't open device \"" << dev << "\": " << errbuf.data() << "!" << std::endl;
        return EXIT_FAILURE;
    }

    int exit_code = EXIT_SUCCESS;

    // Compiled filter program (expression).
    bpf_program fp{};

    try
    {
        std::stringstream ss;
        // Make sure we're capturing on an Ethernet device.
        if (pcap_datalink(handle) != DLT_EN10MB)
        {
            ss << "\"" << dev << "\" is not an Ethernet!" << std::endl;
            throw std::logic_error(ss.str());
        }

        // Compile the filter expression.
        if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1)
        {
            ss << "Couldn't parse filter \"" << filter_exp << "\": " << pcap_geterr(handle) << "!" << std::endl;
            throw std::logic_error(ss.str());
        }

        // Apply the compiled filter.
        if (-1 == pcap_setfilter(handle, &fp))
        {
            ss << "Couldn't install filter \"" << filter_exp << "\": " << pcap_geterr(handle) << "!" << std::endl;
            throw std::logic_error(ss.str());
        }

        if (PCAP_ERROR == pcap_loop(handle, num_packets, pcap_callback, nullptr))
        {
            ss << "Pcap loop failed: " << pcap_geterr(handle) << "!" << std::endl;
            throw std::logic_error(ss.str());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        exit_code = EXIT_FAILURE;
    }
    // Cleanup.
    pcap_freecode(&fp);
    pcap_close(handle);
    if (!exit_code) std::cout << "\nCapture complete." << std::endl;

    return exit_code;
}
