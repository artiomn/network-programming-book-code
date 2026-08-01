#!/usr/bin/env python3

import socket


addr = ('127.0.0.1', 12345)
msg = b'test from client'

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    # s.connect(addr)
    # s.send(msg)
    s.sendto(msg, socket.MSG_FASTOPEN, addr)
    print(s.recv(4096))
