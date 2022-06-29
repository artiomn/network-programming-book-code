#pragma once

#include <cstdint>


// Copied from libpcap to synthesize a PCAP file.
const auto PCAP_VERSION_MAJOR = 2;
const auto PCAP_VERSION_MINOR = 4;
const auto DLT_EN10MB = 1;
const auto BUFFER_SIZE_PKT = 256 * 256 - 1;


struct pcap_timeval
{
	int32_t tv_sec;
	int32_t tv_usec;
};


struct pcap_file_header
{
	const uint32_t magic = 0xa1b2c3d4;
	const uint16_t version_major = PCAP_VERSION_MAJOR;
	const uint16_t version_minor = PCAP_VERSION_MINOR;
	int32_t thiszone = 0;
	uint32_t sigfigs = 0;
	uint32_t snaplen = BUFFER_SIZE_PKT;
	uint32_t linktype = DLT_EN10MB;
};


struct pcap_sf_pkthdr
{
	struct pcap_timeval ts;
	uint32_t caplen;
	uint32_t len;
};


// Various sizes and offsets for our packet read buffer.
const auto BUFFER_SIZE_HDR = sizeof(pcap_sf_pkthdr);
const auto BUFFER_SIZE_ETH = 14;
const auto BUFFER_SIZE_IP = BUFFER_SIZE_PKT - BUFFER_SIZE_ETH;
const auto BUFFER_OFFSET_ETH = sizeof(pcap_sf_pkthdr);
const auto BUFFER_OFFSET_IP = BUFFER_OFFSET_ETH + BUFFER_SIZE_ETH;


#if defined(WIN32)
const auto BUFFER_WRITE_OFFSET = BUFFER_OFFSET_IP;
const auto BUFFER_ADD_HEADER_SIZE = BUFFER_SIZE_ETH;
#else
const auto BUFFER_WRITE_OFFSET = BUFFER_OFFSET_ETH;
const auto BUFFER_ADD_HEADER_SIZE = BUFFER_SIZE_ETH;
#endif
