#include "lwip/debug.h"

#include "lwip/stats.h"

#include "echo.h"

#include <netif/etharp.h>

#include "lwip/tcp.h"
#include "lwip/prot/tcp.h"

#include <iomanip>
#include <iostream>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <time.h>


// Called by echo_msgrecv() when it effectively gets an EOF.
static void echo_msgclose(struct tcp_pcb *pcb)
{
    std::cerr
        << "Closing connection from: "
        << ipaddr_ntoa(&(pcb->remote_ip))
        << std::endl;

    // Remove all the callbacks and shutdown the connection.
    tcp_arg(pcb, nullptr);
    tcp_sent(pcb, nullptr);
    tcp_recv(pcb, nullptr);
    tcp_arg(pcb, nullptr);
    tcp_close(pcb);
}


// Message received callback.
static err_t echo_msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
                          err_t err)
{
    if (ERR_OK == err && p != nullptr)
    {
        struct pbuf *q;

        for (q = p; q != nullptr; q = q->next)
        {
             std::cout
                << "Got: "
                << q->len << "\n"
                << q->payload
                << std::endl;
            // Write echo request to PCB.
            err = tcp_write(pcb, p->payload, p->len, 1);
            if (err != ERR_OK)
            {
                std::cerr
                    << "Echo sending error: "
                    << err
                    << std::endl;
            }

            // Send request immediately.
            err = tcp_output(pcb);
        }

    }
    else if (ERR_OK == err && nullptr == p)
    {
        echo_msgclose(pcb);
    }

    return ERR_OK;
}


// TCP error callback handler.
static void echo_msgerr(void *arg, err_t err)
{
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_msgerr: %s (%i)\n", lwip_strerr(err), err));
    std::cerr
        << "Err: "
        << lwip_strerr(err)
        << std::endl;
}


// TCP accept connection callback handler.
static err_t echo_msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    // Accepted new connection.
    LWIP_PLATFORM_DIAG(("echo_msgaccept called\n"));

    std::cerr
        << "Connect from: "
        << ipaddr_ntoa(&(pcb->remote_ip))
        << ", port: "
        << pcb->remote_port
        << std::endl;

    // Set an arbitrary pointer for callbacks. We don't use this right now.
    //tcp_arg(pcb, esm);

    // Set TCP receive packet callback.
    tcp_recv(pcb, echo_msgrecv);

    // Set an error callback.
    tcp_err(pcb, echo_msgerr);

    return ERR_OK;
}


// Init the TCP listener.
int echo_init(void)
{
    // Create lwIP TCP instance.
    struct tcp_pcb *pcb = tcp_new();
    const short unsigned int port = 11111;

    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init on port %d (pcb: %x)\n", port, pcb));
    int r = tcp_bind(pcb, IP_ADDR_ANY, port);
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init: tcp_bind: %d\n", r));
    // Enable listening.
    pcb = tcp_listen(pcb);
    LWIP_DEBUGF(ECHO_DEBUG, ("echo_init: listen-pcb: %x\n", pcb));
    // Set accept connection callback.
    tcp_accept(pcb, echo_msgaccept);

    return 0;
}


// Hook to incoming TCP packet.
// We catch incoming connections here because the tcp_accept() hook is triggered after the first ACK
int tcp_in_packet(struct tcp_pcb *pcb, struct tcp_hdr *hdr, uint16_t optlen,
                  uint16_t opt1len, uint8_t *opt2, struct pbuf *p)
{
    // First incoming packet is in a LISTEN state.
    if (LISTEN == pcb->state)
    {
        // The tcp_pcb struct does is not filled in with the IP/port details
        // yet, that happens immediately after this callback, so we get these
        // details from other sources. The same sources that are about to fill
        // in the details into the sruct.
        std::cerr
            << "Incomming connection from: "
            << ipaddr_ntoa(ip_current_src_addr())
            << std::endl;
    }
    return ERR_OK;
}

