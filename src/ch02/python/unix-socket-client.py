#!/usr/bin/env python3

import socket

s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM, 0)
s.sendto(b'This is Python!', '_socket_path')
