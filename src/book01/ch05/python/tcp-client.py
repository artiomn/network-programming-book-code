#!/usr/bin/env python3

import argparse
import socket
from array import array

MAX_RECV_BUFFER_SIZE: int = 256


def recv_request(sock: socket.socket) -> bool:  # pylint: disable=redefined-outer-name
    """Receive request from the socket."""

    buffer = array('b', [0] * MAX_RECV_BUFFER_SIZE)

    while True:
        try:
            recv_bytes = sock.recv_into(buffer, len(buffer) - 1, 0)
            print(f'{recv_bytes} was received...')
        except BlockingIOError:
            print('BlockingIOError was caught...')
            return True

        if recv_bytes < 0:
            return False
        if recv_bytes == 0:
            return True

        buffer[recv_bytes] = 0
        print('------------')
        print(buffer.tobytes())


if '__main__' == __name__:
    parser = argparse.ArgumentParser(description='Telnet example.')
    parser.add_argument('host', type=str, help='host name')
    parser.add_argument('port', type=int, default=23, help='telnet port')

    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    sock.connect((args.host, args.port))

    print(f'Connected to "{args.host}"...')

    sock.setblocking(False)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 0)

    while True:
        user_request = input('> ')

        print(f'Sending request: "{user_request}"...')

        user_request += '\n'
        sock.sendall(user_request.encode())

        print('Request was sent, reading response...')
        if not recv_request(sock):
            break
