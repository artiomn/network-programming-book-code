#!/usr/bin/env python3

import socket

sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, socket.IPPROTO_TCP)
sock.connect(('::', 12345))
sock.send(b'test ipv6')
sock.close()
