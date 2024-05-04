#!/usr/bin/env python3

import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
sock.connect(('127.0.0.1', 12345))
sock.send(b'test ipv4')
sock.close()
