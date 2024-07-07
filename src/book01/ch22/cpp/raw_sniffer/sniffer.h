#pragma once

#include <fstream>
#include <string>

// Include winsock headers firstly!
#include <socket_wrapper/socket_headers.h>  // NOLINT

#if !defined(WIN32)
// For ETH_P_ALL, ETH__P_IP, etc...
#    include <netinet/if_ether.h>
#endif

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_wrapper.h>


class Sniffer
{
public:
    explicit Sniffer(
        const std::string &if_name, const std::string &pcap_filename, const socket_wrapper::SocketWrapper &sock_wrap)
        : if_name_(if_name),
          pcap_filename_(pcap_filename),
          sock_wrap_(sock_wrap),
#if defined(WIN32)
          sock_(AF_INET, SOCK_RAW, IPPROTO_IP),
#else
          sock_(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)),
#endif
          // Open and overwrite capture file stream.
          of_(pcap_filename_, std::ios_base::binary)
    {
        if (!sock_)
        {
            throw std::system_error(
                sock_wrap_.get_last_error_code(), std::system_category(),
                "Socket error: " + sock_wrap_.get_last_error_string());
        }

        if (!of_)
        {
            throw std::runtime_error("Failed to open " + pcap_filename_);
        }

        if (!init())
        {
            throw std::runtime_error("Sniffer init failed");
        }
    }

    ~Sniffer();

    Sniffer(const Sniffer &) = delete;
    Sniffer &operator=(const Sniffer &) = delete;
    Sniffer(Sniffer &&) = delete;
    Sniffer &operator=(Sniffer &&) = delete;

public:
    bool initialized() const noexcept { return initialized_; }

public:
    bool start_capture();
    bool stop_capture() noexcept;
    bool switch_promisc(bool enabled) noexcept;
    bool capture();

protected:
    bool init();
    bool bind_socket();
    bool write_pcap_header();

private:
    const std::string if_name_;
    const std::string pcap_filename_;
    const socket_wrapper::SocketWrapper &sock_wrap_;
    const socket_wrapper::Socket sock_;

    std::ofstream of_;
    bool started_ = false;
    bool initialized_ = false;
};
