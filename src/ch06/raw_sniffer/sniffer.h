#pragma once

#include <atomic>
#include <fstream>
#include <string>

// Include winsock headers firstly!
#include <socket_wrapper/socket_headers.h>

#if !defined(WIN32)
//For ETH_P_ALL, ETH__P_IP, etc...
#    include <netinet/if_ether.h>
#endif

#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


class Sniffer
{
public:
    Sniffer(const std::string &if_name, const std::string &pcap_filename, const socket_wrapper::SocketWrapper &sock_wrap) :
        if_name_(if_name), pcap_filename_(pcap_filename), sock_wrap_(sock_wrap),
#if defined(WIN32)
        sock_(AF_INET, SOCK_RAW, IPPROTO_IP),
#else
        sock_(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)),
#endif
        // Open and overwrite capture file stream.
        of_(pcap_filename_, std::ios_base::binary)
    {
        init();
    }

    ~Sniffer();

public:
    bool initialized() const { return initialized_; }

public:
    bool start_capture();
    bool stop_capture();
    bool switch_promisc(bool enabled);
    bool capture();

protected:
    bool init();
    bool bind_socket();
    bool write_pcap_header();

private:
    const size_t ethernet_proto_type_offset = 12;

    const std::string if_name_;
    const std::string pcap_filename_;
    const socket_wrapper::SocketWrapper &sock_wrap_;
    socket_wrapper::Socket sock_;

    std::ofstream of_;
    std::atomic<bool> started_ = false;
    bool initialized_ = false;
};
