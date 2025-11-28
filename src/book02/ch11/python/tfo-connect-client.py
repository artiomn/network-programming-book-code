#!/usr/bin/env python3

import socket


TCP_FASTOPEN_CONNECT = 30

addr = ('127.0.0.1', 12345)
msg = b'test from client'

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_TCP, TCP_FASTOPEN_CONNECT, 1)
    s.connect(addr)
    s.send(msg)
    print(s.recv(4096))
