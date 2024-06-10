#!/usr/bin/env python3

from ping3 import ping, verbose_ping


if '__main__' == __name__:
    result = ping('google.com')
    print(f'Ping: {result}')
    verbose_ping('google.com', count=5)
