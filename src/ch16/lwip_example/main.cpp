#include <iostream>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <pcap/pcap.h>

#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/ethip6.h>
#include <netif/etharp.h>
#include <lwip/udp.h>
#include <lwip/mld6.h>
#include <lwip/timeouts.h>

#include "echo.h"
#include "ping.h"


// Callback to send a raw lwIP packet via PCAP.
static err_t pcap_output(struct netif *netif, struct pbuf *p)
{
    pcap_t *pcap = reinterpret_cast<pcap_t *>(netif->state);
    std::cout
        << "Sending packet with length "
        << p->tot_len
        << std::endl;

    // Just fire the raw packet data down PCAP.
    int r = pcap_sendpacket(pcap, reinterpret_cast<const uint8_t *>(p->payload), p->tot_len);

    if (r != 0)
    {
        std::cerr
            << "Error sending packet\n"
            << "Error: " << pcap_geterr(pcap)
            << std::endl;
        return ERR_IF;
    }

    return ERR_OK;
}


// Raw input packet callback.
static err_t input_callback(struct pbuf *p, struct netif *inp)
{
    // Start of payload will have an Ethernet header.
    struct eth_hdr *ethhdr = reinterpret_cast<struct eth_hdr *>(p->payload);

    // "src" contains the source hardware address from the packet.
    struct eth_addr *ethaddr = &ethhdr->src;

    std::cout << "OK" << std::endl;

    // Pass the packet to the regular internal lwIP input callback.
    return netif_input(p, inp);
}


// The lwIP network interface init callback.
static err_t init_callback(struct netif *netif)
{
    netif->name[0] = 't';
    netif->name[1] = 'p';

    // Raw packet is ready to be transmitted, use the callback above.
    netif->linkoutput = pcap_output;
    // Use the normal lwIP internal callback for preparing a packet
    // header for transmission.
    netif->output = etharp_output;
    // Use the input callback above for packets coming in.
    netif->input = input_callback;

    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;

    netif_set_link_up(netif);

    return ERR_OK;
}


int main(int argc, const char * const argv[])
{
    const char * const interface = (argc >= 2) ? argv[1] : "eth0";

    std::cout
        << "Opening interface \""
        << interface << "\""
        << std::endl;

    // Open PCAP on some interface.
    pcap_t *pcap = pcap_open_live(interface, 65536, 1, 100, nullptr);
    char errbuf[PCAP_ERRBUF_SIZE];

    // Create a lwIP network interface struct and give it a MAC.
    struct netif netif = { .hwaddr_len = 6, 0 };
    memcpy(netif.hwaddr, "\xaa\x00\x00\x00\x00\x01", 6);

    // This is the hard-coded listen IP address.
    ip4_addr_t ip, mask, gw;
    IP4_ADDR(&ip, 172, 20, 20, 5);
    IP4_ADDR(&mask, 255, 255, 0, 0);
    IP4_ADDR(&gw, 172, 20, 20, 1);

    // Add the lwIP network interface to PCAP with a couple of callbacks.
    netif_add(&netif, &ip, &mask, &gw, pcap, init_callback, ethernet_input);
    netif_set_up(&netif);

    NETIF_SET_CHECKSUM_CTRL(&netif, 0x00FF);

    // Initialize TCP listener.
    if (echo_init() != 0)
    {
        std::cerr << "TCP init failure" << std::endl;
        return EXIT_FAILURE;
    }

    // Initialize ICMP listener.
    if (ping_init() != 0)
    {
        std::cerr << "ICMP init failure" << std::endl;
        return EXIT_FAILURE;
    }

    struct pcap_pkthdr *hdr = nullptr;
    const unsigned char *data = nullptr;

    while (true)
    {
        // Get next PCAP message (blocking).
        int r = pcap_next_ex(pcap, &hdr, &data);

        switch (r)
        {
            case 0:
                // Timeout
                continue;

            case -1:
                std::cerr
                    << "Error: "
                    << pcap_geterr(pcap)
                    << std::endl;
                continue;

            case 1:
                break;

            default:
                std::cerr
                    << "Unknown result: " << r
                    << std::endl;
                continue;
        }

        // Copy the packet to lwIP's packet buffer and trigger a lwIP packet input.
        std::cout
            << "Packet length: "
            << hdr->len << "/"
            << hdr->caplen
            << std::endl;
        struct pbuf *pbuf = pbuf_alloc(PBUF_RAW, hdr->len, PBUF_RAM);
        memcpy(pbuf->payload, data, hdr->len);
        netif.input(pbuf, &netif);
    }

    return EXIT_SUCCESS;
}

