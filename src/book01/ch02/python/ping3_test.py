#!/usr/bin/env python3

from ping3 import ping, verbose_ping


if '__main__' == __name__:
    print(f'Ping: {ping("google.com")}')
    verbose_ping('google.com', count=5)
