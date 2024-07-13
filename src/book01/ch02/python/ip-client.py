#!/usr/bin/env python3

import socket


sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.sendto(b'test ipv6', ('::', 12345))
sock.close()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.sendto(b'test ipv4', ('127.0.01', 12345))
sock.close()
