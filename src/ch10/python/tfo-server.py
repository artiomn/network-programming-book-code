#!/usr/bin/env python3

import socket


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_TCP, socket.TCP_FASTOPEN, 5)
    s.bind(('', 12345))
    s.listen(1)

    while True:
        with s.accept()[0] as conn:
            print(conn.recv(4096))
            conn.send(b'test from server')
