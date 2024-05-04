#!/usr/bin/env python3

import scapy.all as scapy


packet = scapy.IP(dst='192.168.1.1') / scapy.TCP(dport=22) / 'EXAMPLE'
packet.summary()

scapy.ls(packet)
scapy.hexdump(packet)

reponse = scapy.sr(packet)
