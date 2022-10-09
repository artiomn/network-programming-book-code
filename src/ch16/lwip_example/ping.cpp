#include "lwip/debug.h"

#include "ping.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"

#include <iostream>


// ICMP message received. Return 0 to let lwIP process it and 1 to eat the
// packet.
static u8_t ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_ASSERT("p != NULL", p != nullptr);

    std::cout
        << "Ping accepted from "
        << ipaddr_ntoa(addr)
        << std::endl;

    struct icmp_echo_hdr *iecho = nullptr;
    // If the message is long enough, get the header and do something.
    if (p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr)))
    {
        iecho = reinterpret_cast<struct icmp_echo_hdr *>(p->payload + PBUF_IP_HLEN);
    }

    return 0;
}


// Initialise the ICMP hooks.
int ping_init(void)
{
    struct raw_pcb *ping_pcb;
    // Create a new listener instance for ICMP messages.
    ping_pcb = raw_new(IP_PROTO_ICMP);
    LWIP_ASSERT("ping_pcb != NULL", ping_pcb != nullptr);

    // Callback to ping_recv in ICMP message received.
    raw_recv(ping_pcb, ping_recv, nullptr);

    // Bind to ICMP on any address.
    raw_bind(ping_pcb, IP_ADDR_ANY);

    std::cout << "Ping initialized..." << std::endl;

    return 0;
}
