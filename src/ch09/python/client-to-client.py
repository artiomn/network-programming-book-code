#!/usr/bin/env python

import socket
import sys
import time


if len(sys.argv) < 3:
    print('Usage: c2c <local_port> <remote_address:port>')
    sys.exit(1)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP) as sock:
    sock.bind(('0.0.0.0', int(sys.argv[1])))

    a, p = f'{sys.argv[2]}'.split(':')

    time.sleep(5)

    while True:
        try:
            print('Trying to connect...')
            sock.connect((a, int(p)))
            print('Connected!')
            break
        except ConnectionRefusedError:
            time.sleep(0.01)
            continue

    while True:
        data = f'I\'am {sock.getsockname()} sending data to ' f'{sock.getpeername()}'
        print(f'Sending "{data}"')
        sock.send(data.encode())
        print('Receiving...')
        recv_data = sock.recv(len(data))
        print(f'Received from {sock.getpeername()}: {recv_data.decode()}')
        time.sleep(1)
