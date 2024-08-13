#!/usr/bin/python3

import socket


ip4 = socket.inet_pton(socket.AF_INET, '10.12.110.57')
ip6 = socket.inet_pton(socket.AF_INET6, '2001:db8:63b3:1::3490')

print(socket.inet_ntop(socket.AF_INET, ip4))
print(socket.inet_ntop(socket.AF_INET6, ip6))
