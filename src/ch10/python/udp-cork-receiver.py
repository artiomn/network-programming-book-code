#!/usr/bin/env python3

import socket


with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    i = 0
    s.bind(('0.0.0.0', 1345))
    while True:
        data = s.recv(4096)
        print(f'{i}: {data.decode()}')
        i += 1
