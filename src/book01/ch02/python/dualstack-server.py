#!/usr/bin/env python3

import socket

with socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as sock:
    sock.bind(('::', 12345, 0, 0))

    print(f'Listening socket = {sock}, ' f'dual stack = {socket.has_dualstack_ipv6()}\n')

    while True:
        while data := sock.recvfrom(15):
            print(data)
