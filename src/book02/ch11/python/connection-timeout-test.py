#!/usr/bin/env python3

import socket


with socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP) as sock:
    try:
        sock.settimeout(1.5)
        sock.connect(('8.8.8.8', 12345))
    except TimeoutError:
        print('Trying to connect to another host...')
        sock.shutdown(socket.SHUT_RDWR)
        sock.connect(('ya.ru', socket.getservbyname('https')))
    print(f'Connected {socket.getnameinfo(sock.getpeername(), 0)}!')
