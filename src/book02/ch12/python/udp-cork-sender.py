#!/usr/bin/env python3

import socket


UDP_CORK = 1

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    s.setsockopt(socket.SOL_UDP, UDP_CORK, 1)
    for i in range(10):
        data = f'string{i}'
        print(data)
        s.sendto(data.encode(), ('localhost', 1345))
    s.setsockopt(socket.SOL_UDP, UDP_CORK, 0)
