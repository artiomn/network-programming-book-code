#!/usr/bin/env python3
# pylint: disable=R0801(duplicate-code)

import zmq


context = zmq.Context()

sock = context.socket(zmq.REP)
sock.bind('tcp://127.0.0.1:12345')

while True:
    message = sock.recv()
    sock.send(f'Echo: {message}'.encode())
    print(f'Echo: {message}')
