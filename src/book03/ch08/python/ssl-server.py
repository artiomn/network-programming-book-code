#!/usr/bin/python3

import socket
import ssl
from pathlib import Path

cur_dir = Path(__file__).parent


context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain(cur_dir / 'server.pem', cur_dir / 'key.pem')

with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
    sock.bind(('127.0.0.1', 1443))
    sock.listen(5)
    with context.wrap_socket(sock, server_side=True) as ssl_sock:
        conn, addr = ssl_sock.accept()

        print(conn.version(), addr)
        data = conn.recv()
        print(f'Data = {data.decode()}')
        conn.sendall(f'SSL test {data.decode()}'.encode())
