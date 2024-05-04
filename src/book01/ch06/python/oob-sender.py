#!/usr/bin/env python3

import socket
import time


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
sock.connect(('127.0.0.1', 12345))

time.sleep(0.1)

sock.send(b'abcd', socket.MSG_OOB)
