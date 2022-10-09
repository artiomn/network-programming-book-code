#include <iomanip>
#include <iostream>

#include "packet_printer.h"

// Ethernet headers are always exactly 14 bytes.
const auto SIZE_ETHERNET = 14;

// Ethernet addresses are 6 bytes.
const auto ETHER_ADDR_LEN = 6;

// Ethernet header.
struct sniff_ethernet
{
    u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
    u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
    u_short ether_type;                     /* IP? ARP? RARP? etc */
};


// IP header.
struct sniff_ip
{
    u_char  ip_vhl;         // Version << 4 | header length >> 2.
    u_char  ip_tos;         // Type of service.
    u_short ip_len;         // Total length.
    u_short ip_id;          // Identification.
    u_short ip_off;         /* fragment offset field */
    const u_short ip_rf = 0x8000;        /* reserved fragment flag */
    const u_short ip_df = 0x4000;        /* don't fragment flag */
    const u_short ip_mf = 0x2000;        /* more fragments flag */
    const u_short ip_offmask = 0x1fff;       /* mask for fragmenting bits */
    u_char  ip_ttl;         /* time to live */
    u_char  ip_p;           /* protocol */
    u_short ip_sum;         /* checksum */
    in_addr ip_src;
    in_addr ip_dst;  /* source and dest address */
};


constexpr u_char ip_hl(const sniff_ip *ip)
{
    return (ip->ip_vhl) & 0x0f;
}


constexpr u_char ip_v(const sniff_ip *ip)
{
    return (ip->ip_vhl) >> 4;
}


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
    u_char  th_offx2;
    u_char  th_flags;
    const u_char th_fin = 0x01;
    const u_char th_syn = 0x02;
    const u_char th_rst = 0x04;
    const u_char th_push = 0x08;
    const u_char th_ack_f = 0x10;
    const u_char th_urg = 0x20;
    const u_char th_ece = 0x40;
    const u_char th_cwr = 0x80;
    const u_char th_flags2 = (th_fin|th_syn|th_rst|th_ack_f|th_urg|th_ece|th_cwr);
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

    int i;
    int gap;
    const u_char *ch;

    /* offset */
    std::cout << std::setw(5) << offset << std::endl;

    /* hex */
    ch = payload;
    for(i = 0; i < len; ++i)
    {
        std::cout << std::setw(2) << std::hex << *ch++ << " ";
        /* print extra space after 8th byte for visual aid */
        if (7 == i) std::cout << " ";
    }

    std::cout << std::resetiosflags(std::ios_base::basefield);

    /* print space to handle line less than 8 bytes */
    if (len < 8) std::cout << " ";

    /* fill hex gap with spaces if not full line */
    if (len < 16)
    {
        gap = 16 - len;
        for (i = 0; i < gap; ++i)
        {
            std::cout << "   ";
        }
    }
    std::cout << "   ";

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; ++i)
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
    int line_width = 16;            /* number of bytes per line */
    int line_len;
    int offset = 0;                    /* zero-based offset counter */
    const u_char *ch = payload;

    if (len <= 0) return;

    /* data fits on one line */
    if (len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    // Data spans multiple lines.
    for (;;)
    {
        /* compute current line length */
        line_len = line_width % len_rem;
        /* print line */
        print_hex_ascii_line(ch, line_len, offset);
        /* compute total remaining */
        len_rem = len_rem - line_len;
        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;
        offset = offset + line_width;
        /* check if we have line width chars or less */
        if (len_rem <= line_width)
        {
            /* print last line and get out */
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

    const sniff_ethernet *ethernet = reinterpret_cast<const sniff_ethernet*>(packet);
    const sniff_ip *ip = reinterpret_cast<const sniff_ip*>(packet + SIZE_ETHERNET);
    const sniff_tcp *tcp;
    // Packet payload.
    const char *payload;

    /* define/compute ip header offset */
    int size_ip = ip_hl(ip) * 4;
    int size_tcp;
    int size_payload;

    std::cout << "\nPacket number: " << packet_count_++ << std::endl;

    if (size_ip < 20)
    {
        std::cout << "   * Invalid IP header length: " << size_ip <<  " bytes" << std::endl;
        return;
    }

    /* print source and destination IP addresses */
    std::cout << "       From: " << inet_ntoa(ip->ip_src) << std::endl;
    std::cout << "       To: " << inet_ntoa(ip->ip_dst) << std::endl;

    /* determine protocol */
    switch(ip->ip_p)
    {
        case IPPROTO_TCP:
            std::cout << "   Protocol: TCP" << std::endl;
        break;
        case IPPROTO_UDP:
            std::cout << "   Protocol: UDP" << std::endl;
        return;
        case IPPROTO_ICMP:
            std::cout << "   Protocol: ICMP" << std::endl;
        return;
        case IPPROTO_IP:
            std::cout << "   Protocol: IP" << std::endl;
        return;
        default:
            std::cout << "   Protocol: unknown" << std::endl;
        return;
    }

    // OK, this packet is TCP.

    /* define/compute tcp header offset */
    tcp = reinterpret_cast<const sniff_tcp*>(packet + SIZE_ETHERNET + size_ip);
    size_tcp = th_off(tcp) * 4;
    if (size_tcp < 20)
    {
        std::cerr << "   * Invalid TCP header length: " << size_tcp << " bytes"  << std::endl;
        return;
    }

    std::cout
        << "   Src port: " <<  ntohs(tcp->th_sport) << "\n"
        << "   Dst port: " << ntohs(tcp->th_dport)
        << std::endl;

    /* define/compute tcp payload (segment) offset */
    payload = (const char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

    /*
     * Print payload data; it might be binary, so don't just
     * treat it as a string.
     */
    if (size_payload > 0)
    {
        std::cout << "   Payload (" << size_payload << " bytes):\n";
        print_payload(reinterpret_cast<const unsigned char*>(payload), size_payload);
        std::cout << std::endl;
    }
}
