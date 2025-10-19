#include "packet_printer.h"

#include <iomanip>
#include <iostream>
#include <string>

#if !defined(ETHERTYPE_IP)
constexpr auto ETHERTYPE_IP = 0x0800;
#endif

#if !defined(ETHERTYPE_IPV6)
constexpr auto ETHERTYPE_IPV6 = 0x86DD;
#endif

// Ethernet headers are always exactly 14 bytes.
constexpr auto SIZE_ETHERNET = 14;

// Ethernet addresses are 6 bytes.
constexpr auto ETHER_ADDR_LEN = 6;

// IP header size in bytes.
constexpr auto ip_header_size = 20;

// Ethernet header.
struct sniff_ethernet
{
    u_char ether_dhost[ETHER_ADDR_LEN]; /* destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* source host address */
    u_short ether_type;                 /* IP? ARP? RARP? etc */
};

// Reserved fragment flag.
constexpr u_short ip_rf = 0x8000;
// don't fragment flag.
constexpr u_short ip_df = 0x4000;
// more fragments flag.
constexpr u_short ip_mf = 0x2000;
// mask for fragmenting bits.
constexpr u_short ip_offmask = 0x1fff;

// IP header.
struct sniff_ip
{
    // Version << 4 | header length >> 2.
    u_char ip_vhl;
    // Type of service.
    u_char ip_tos;
    // Total length.
    u_short ip_len;
    u_short ip_id;  // Identification.
    u_short ip_off; /* fragment offset field */
    u_char ip_ttl;  /* time to live */
    u_char ip_p;    /* protocol */
    u_short ip_sum; /* checksum */
    in_addr ip_src;
    in_addr ip_dst; /* source and dest address */
};


struct sniff_ipv6
{
    u_int version_class_flow;
    u_short payload_len;
    u_char next_header;
    u_char hop_limit;
    in6_addr src;
    in6_addr dest;
};


constexpr u_char ip_hl(const sniff_ip *ip)
{
    return (ip->ip_vhl) & 0x0f;
}


constexpr u_char ip_v(const sniff_ip *ip)
{
    return (ip->ip_vhl) >> 4;
}


enum class tcp_flags : u_char
{
    th_fin = 0x01,
    th_syn = 0x02,
    th_rst = 0x04,
    th_push = 0x08,
    th_ack_f = 0x10,
    th_urg = 0x20,
    th_ece = 0x40,
    th_cwr = 0x80
};


// TCP header.
struct sniff_tcp
{
    typedef u_int tcp_seq;
    // Source port.
    u_short th_sport;
    // Destination port.
    u_short th_dport;
    // Sequence number.
    tcp_seq th_seq;
    // Acknowledgement number.
    tcp_seq th_ack;
    // Data offset, rsvd.
    u_char th_offx2;
    // cppcheck-suppress unusedStructMember
    tcp_flags th_flags;
    u_short th_win;
    u_short th_sum;
    u_short th_urp;
};


constexpr u_char th_off(const sniff_tcp *th)
{
    return (th->th_offx2 & 0xf0) >> 4;
}


void PacketPrinter::print_hex_ascii_line(const u_char *payload, int len, int offset)
{
    // Offset.
    std::cout << std::setw(5) << offset << std::endl;

    // Hex.
    const u_char *ch = payload;
    for (int i = 0; i < len; ++i)
    {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << +*ch++ << " ";
        // Print extra space after 8th byte for visual aid.
        if (7 == i) std::cout << " ";
    }

    std::cout << std::resetiosflags(std::ios_base::basefield);

    // Print space to handle line less than 8 bytes.
    if (len < 8) std::cout << " ";

    // Fill hex gap with spaces if not full line.
    if (len < 16)
    {
        int gap = 16 - len;
        for (int i = 0; i < gap; ++i)
        {
            std::cout << "   ";
        }
    }
    std::cout << "   ";

    // ASCII (if printable).
    ch = payload;
    for (int i = 0; i < len; ++i)
    {
        if (isprint(*ch))
            std::cout << *ch++;
        else
            std::cout << ".";
    }

    std::cout << std::endl;
}


// Print packet payload data (avoid printing binary data).
void PacketPrinter::print_payload(const u_char *payload, int len)
{
    int len_rem = len;
    // Number of bytes per line.
    constexpr int line_width = 16;
    // Zero-based offset counter.
    int offset = 0;
    const u_char *ch = payload;

    if (len <= 0) return;

    // Data fits on one line.
    if (len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    // Data spans multiple lines.
    for (;;)
    {
        // Compute current line length.
        int line_len = line_width % len_rem;
        // Print line.
        print_hex_ascii_line(ch, line_len, offset);
        // Compute total remaining.
        len_rem = len_rem - line_len;
        // Shift pointer to remaining bytes to print.
        ch = ch + line_len;
        offset = offset + line_width;
        // Check if we have line width chars or less.
        if (len_rem <= line_width)
        {
            // Print last line and get out.
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }
}


// Dissect/print packet
void PacketPrinter::got_packet(u_char *args, const pcap_pkthdr *header, const u_char *packet)
{
    (void)args;
    (void)header;

    std::cout << "\nPacket number: " << packet_count_++ << std::endl;
    const sniff_ethernet *ethernet = reinterpret_cast<const sniff_ethernet *>(packet);

    std::string proto_addr;
    int network_proto_type;
    int transport_proto_type;
    const void *src, *dst;
    size_t size_ip;
    size_t ip_len;


    switch (ntohs(ethernet->ether_type))
    {
        case ETHERTYPE_IP:
        {
            std::cout << "  IPv4 protocol" << std::endl;
            proto_addr.resize(INET_ADDRSTRLEN);
            network_proto_type = AF_INET;

            const sniff_ip *ip = reinterpret_cast<const sniff_ip *>(packet + SIZE_ETHERNET);

            size_ip = ip_hl(ip) * 4;
            ip_len = ntohs(ip->ip_len);

            if (size_ip < ip_header_size)
            {
                std::cout << "   * Invalid IP header length: " << size_ip << " bytes" << std::endl;
                return;
            }

            transport_proto_type = ip->ip_p;
            src = &ip->ip_src;
            dst = &ip->ip_dst;
            break;
        }
        case ETHERTYPE_IPV6:
        {
            std::cout << "  IPv6 protocol" << std::endl;
            proto_addr.resize(INET6_ADDRSTRLEN);
            network_proto_type = AF_INET6;

            const sniff_ipv6 *ip = reinterpret_cast<const sniff_ipv6 *>(packet + SIZE_ETHERNET);

            size_ip = sizeof(sniff_ipv6);
            ip_len = ntohs(ip->payload_len);

            transport_proto_type = ip->next_header;
            src = &ip->src;
            dst = &ip->dest;
            break;
        }
        default:
        {
            std::cout << "  Unknown EtherType = 0x" << std::hex << ntohs(ethernet->ether_type) << std::endl;
            return;
        }
    }

    std::cout << std::dec << "    From: " << inet_ntop(network_proto_type, src, &proto_addr[0], proto_addr.size())
              << "\n"
              << "    To: " << inet_ntop(network_proto_type, dst, &proto_addr[0], proto_addr.size()) << std::endl;

    // Determine protocol.
    switch (transport_proto_type)
    {
        case IPPROTO_TCP:
            std::cout << "    Protocol: TCP" << std::endl;
            break;
        case IPPROTO_UDP:
            std::cout << "    Protocol: UDP" << std::endl;
            return;
        case IPPROTO_ICMP:
            std::cout << "    Protocol: ICMP" << std::endl;
            return;
        case IPPROTO_ICMPV6:
            std::cout << "    Protocol: ICMPv6" << std::endl;
            return;
        case IPPROTO_IP:
            std::cout << "    Protocol: IP" << std::endl;
            return;
        case IPPROTO_IPV6:
            std::cout << "    Protocol: IPv6" << std::endl;
            return;
        default:
            std::cout << "    Protocol: unknown [" << transport_proto_type << "]" << std::endl;
            return;
    }

    // OK, this packet is TCP.

    // Define/compute tcp header offset.
    const sniff_tcp *tcp = reinterpret_cast<const sniff_tcp *>(packet + SIZE_ETHERNET + size_ip);
    const int size_tcp = th_off(tcp) * 4;
    if (size_tcp < ip_header_size)
    {
        std::cerr << "    * Invalid TCP header length: " << size_tcp << " bytes" << std::endl;
        return;
    }

    std::cout << "    Src port: " << ntohs(tcp->th_sport) << "\n"
              << "    Dst port: " << ntohs(tcp->th_dport) << std::endl;

    // Define/compute tcp payload (segment) offset.
    const u_char *payload = static_cast<const u_char *>(packet + SIZE_ETHERNET + size_ip + size_tcp);

    // Compute tcp payload (segment) size.
    const int size_payload = ip_len - (size_ip + size_tcp);

    // Print payload data; it's usually binary, so don't just
    // treat it as a string.
    if (size_payload > 0)
    {
        std::cout << "    Payload (" << size_payload << " bytes):\n";
        print_payload(reinterpret_cast<const unsigned char *>(payload), size_payload);
        std::cout << std::endl;
    }
}
