#!/usr/bin/env python3

import socket


# localhost.localdomain
print(socket.getfqdn())

# unknowndomain.com
print(socket.getfqdn('unknowndomain.com'))

# yandex.ru
print(socket.getfqdn('yandex.ru'))

# localhost.localdomain
print(socket.getfqdn('0.0.0.0'))

# dns.google
print(socket.getfqdn('8.8.8.8'))
