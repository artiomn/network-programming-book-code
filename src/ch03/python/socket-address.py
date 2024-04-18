#!/usr/bin/env python3

import socket


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
sock.connect(('google.com', 443))

print(sock.getsockname())
print(sock.getpeername())
