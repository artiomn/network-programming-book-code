#!/usr/bin/env python3

import pcapy


dump = pcapy.open_offline('dump.pcap')
dumper = dump.dump_open('new_dump.pcap')

hdr, body = dump.next()

while hdr is not None:
    dumper.dump(hdr, body)
    print(len(body))
    hdr, body = dump.next()
