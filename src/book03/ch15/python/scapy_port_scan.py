#!/usr/bin/env python3

import sys
from scapy.all import IP, TCP, sr


if len(sys.argv) != 2:
    print(f'{sys.argv[0]} <IP>')
    sys.exit(1)

packet = IP(dst=sys.argv[1]) / TCP(dport=(1, 1234), flags='A')

answers, _ = sr(packet, timeout=10)

answers.summary()

for a in answers:
    print(a[1].sport, a[1].dport)
