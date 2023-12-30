#!/usr/bin/env python3

import sys
import zmq


context = zmq.Context()

sock = context.socket(zmq.REQ)
sock.connect('tcp://localhost:12345')

sock.send(' '.join(sys.argv[1:]).encode())
print(sock.recv())
