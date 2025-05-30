#pragma once

#include <cstdint>


// Copied with modifications from libpcap to synthesize a PCAP file.
constexpr auto PCAP_VERSION_MAJOR = 2;
constexpr auto PCAP_VERSION_MINOR = 4;
constexpr auto DLT_EN10MB = 1;
constexpr auto BUFFER_SIZE_PKT = 256 * 256 - 1;


struct pcap_timeval
{
    // cppcheck-suppress unusedStructMember
    int32_t tv_sec;
    // cppcheck-suppress unusedStructMember
    int32_t tv_usec;
};


struct pcap_file_header
{
    // cppcheck-suppress unusedStructMember
    const uint32_t magic = 0xa1b2c3d4;
    // cppcheck-suppress unusedStructMember
    const uint16_t version_major = PCAP_VERSION_MAJOR;
    // cppcheck-suppress unusedStructMember
    const uint16_t version_minor = PCAP_VERSION_MINOR;
    int32_t thiszone = 0;
    uint32_t sigfigs = 0;
    uint32_t snaplen = BUFFER_SIZE_PKT;
    uint32_t linktype = DLT_EN10MB;
};


struct pcap_sf_pkthdr
{
    pcap_timeval ts;
    uint32_t caplen;
    uint32_t len;
};


// Various sizes and offsets for our packet read buffer.
constexpr auto BUFFER_SIZE_HDR = sizeof(pcap_sf_pkthdr);
constexpr auto BUFFER_SIZE_ETH = 14;
constexpr auto BUFFER_SIZE_IP = BUFFER_SIZE_PKT - BUFFER_SIZE_ETH;
constexpr auto BUFFER_OFFSET_ETH = sizeof(pcap_sf_pkthdr);
constexpr auto BUFFER_OFFSET_IP = BUFFER_OFFSET_ETH + BUFFER_SIZE_ETH;


#if defined(WIN32)
constexpr auto BUFFER_WRITE_OFFSET = BUFFER_OFFSET_IP;
constexpr auto BUFFER_ADD_HEADER_SIZE = BUFFER_SIZE_ETH;
#else
constexpr auto BUFFER_WRITE_OFFSET = BUFFER_OFFSET_ETH;
constexpr auto BUFFER_ADD_HEADER_SIZE = BUFFER_SIZE_ETH;
#endif
