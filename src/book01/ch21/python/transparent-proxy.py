#!/usr/bin/env python3

import socket


IP_TRANSPARENT = 0x13


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.IPPROTO_IP, IP_TRANSPARENT, 1)

    sock.bind(('8.8.8.8', 1234))
    sock.listen()

    print(f'Listening on {sock.getsockname()}')
    while True:
        cln_sock, (remote_ip, remote_port) = sock.accept()
        local_ip, local_port = cln_sock.getsockname()
        print(f'Connection from {remote_ip}:{remote_port} to {local_ip}:{local_port}')
        cln_sock.close()
