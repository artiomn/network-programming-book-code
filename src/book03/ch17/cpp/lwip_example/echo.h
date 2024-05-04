#pragma once

#include <stdint.h>

struct pbuf;
struct tcp_pcb;
struct tcp_hdr;

int echo_init(void);
int tcp_in_packet(struct tcp_pcb *pcb, struct tcp_hdr *hdr, uint16_t optlen, uint16_t opt1len, uint8_t *opt2, struct pbuf *p);
