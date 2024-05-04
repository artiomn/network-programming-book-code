#!/usr/bin/env python3

import socket
import sys


with socket.create_server(('0.0.0.0', 12345)) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVLOWAT, int(sys.argv[1]))
    i = 0
    with s.accept()[0] as client:
        data = client.recv(4096)
        while data:
            print(i, data.decode().rstrip())
            data = client.recv(4096)
            i += 1
