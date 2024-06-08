#!/usr/bin/env python3

import socket
import sys

IP_RECVTTL = 12


if len(sys.argv) < 2:
    print(f'{sys.argv[0]} <port>')
    sys.exit(1)

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind(('', int(sys.argv[1])))

    sock.setsockopt(socket.IPPROTO_IP, IP_RECVTTL, 1)

    ttl = 0
    ttl_size = 4

    msg, ancdata, flags, addr = sock.recvmsg(255, socket.CMSG_LEN(ttl_size))

    print(f'{len(msg)} bytes was read: {msg.decode()}')
    for cmsg_level, cmsg_type, cmsg_data in ancdata:
        if cmsg_level == socket.IPPROTO_IP and socket.IP_TTL == cmsg_type:
            ttl_pointer = cmsg_data[: len(cmsg_data) - (len(cmsg_data) % ttl_size)]

            ttl = int.from_bytes(ttl_pointer, byteorder='little')
            print(f'IP_RECVTTL was received: {ttl}')
            break
