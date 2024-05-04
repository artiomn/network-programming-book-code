#!/usr/bin/env python3

import socket

with socket.socket(socket.AF_INET6, socket.SOCK_STREAM, socket.IPPROTO_TCP) as sock:
    sock.bind(('::', 12345, 0, 0))
    sock.listen()
    print(f'Listening socket = {sock}, ' f'dual stack = {socket.has_dualstack_ipv6()}\n')

    while True:
        s, addr = sock.accept()
        print(f'Client socket = {s}\nClient address = {addr}')

        with s:
            while data := s.recv(15):
                print(data)
