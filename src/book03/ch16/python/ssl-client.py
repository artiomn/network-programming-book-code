#!/usr/bin/env python3

import socket
import ssl


hostname = 'www.google.com'
context = ssl.create_default_context()

with socket.create_connection((hostname, 443)) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as tls_sock:
        print(tls_sock.version())
        tls_sock.sendall(b'GET / HTTP/1.1\n\n')
        while data := tls_sock.recv():
            print(data.decode())
