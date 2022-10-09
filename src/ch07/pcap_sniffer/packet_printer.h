#pragma once

#include <pcap.h>


class PacketPrinter
{
public:
    // Dissect/print packet
    void got_packet(u_char *args, const pcap_pkthdr *header, const u_char *packet);

private:
    // Print data in rows of 16 bytes: offset   hex   ascii
    // 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
    void print_hex_ascii_line(const u_char *payload, int len, int offset);

    // Print packet payload data (avoid printing binary data).
    void print_payload(const u_char *payload, int len);

private:
    int packet_count_ = 1;
};

