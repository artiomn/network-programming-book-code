#!/usr/bin/env python3

from pythonping import ping


if '__main__' == __name__:
    print(f'Ping:\n{ping("google.com")}')
