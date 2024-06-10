#!/usr/bin/env python3

from pythonping import ping


if '__main__' == __name__:
    result = ping('google.com')
    print(f'Ping:\n{result}')
