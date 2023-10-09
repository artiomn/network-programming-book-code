#!/usr/bin/env python

import curio
from curio import socket


async def udp_echo(addr):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(addr)
    while True:
        data, addr = await sock.recvfrom(1000)
        print('Received from', addr, data)
        await sock.sendto(data, addr)


if '__main__' == __name__:
    curio.run(udp_echo, ('', 12345))
