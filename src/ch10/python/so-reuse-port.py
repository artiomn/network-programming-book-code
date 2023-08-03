#!/usr/bin/env python3

import socket


srv1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
srv1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
srv1.bind(('192.168.2.13', 12345))
srv1.listen()

srv2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
srv2.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
srv2.bind(('192.168.2.13', 12345))
srv2.listen()

input()
