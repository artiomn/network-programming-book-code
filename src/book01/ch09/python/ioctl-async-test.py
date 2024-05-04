#!/usr/bin/env python3
# mypy: ignore-errors

import fcntl
import socket
import sys
import time
from termios import FIONBIO

if len(sys.argv) < 3:
    print(f'{sys.argv[0]} <port> <async_mode_flag>')
    sys.exit(1)

port = int(sys.argv[1])

async_mode = int(sys.argv[2].lower() in ['true', 't', '1', 'y'])

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

    sock.bind(('', port))
    sock.listen()

    print(f'Async mode = {async_mode}')

    if callable(getattr(sock, 'ioctl', None)):
        print('Using socket.ioctl() method...')
        sock.ioctl(FIONBIO, async_mode)
    else:
        print('Using fcntl.ioctl() function...')
        buf = int(async_mode).to_bytes(length=4, byteorder='little')
        io_result = fcntl.ioctl(sock.fileno(), FIONBIO, buf)
        print(f'io_result = {io_result}')

        while True:
            try:
                with sock.accept()[0] as c_sock:
                    break
            except BlockingIOError:
                print('Client not connected.')
                time.sleep(1)

        print('Client connected!')
