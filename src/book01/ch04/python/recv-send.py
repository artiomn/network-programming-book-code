#!/usr/bin/env python3

import socket


buffer_size = 255

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    s.bind(('0.0.0.0', 8080))

    while True:
        data, client_address = s.recvfrom(buffer_size)

        if data:
            s.sendto(data, client_address)

# pylint: disable=broad-except
except Exception as e:
    print(f'Socket error: {e}')
