#!/usr/bin/python3

import os
import socket
import sys
import time


def print_ips_with_getaddrinfo(host_name: str):
    print(f'Getting name for "{host_name}"...\nUsing getaddrinfo() function.')

    addrs = socket.getaddrinfo(
        host_name, None, family=socket.AF_UNSPEC, type=socket.SOCK_STREAM, flags=socket.AI_CANONNAME, proto=0
    )

    for (ai_family, _, _, ai_canonname, ai_addr) in addrs:
        print(
            f'Canonical name: {ai_canonname}\n'
            f'Address type: '
            f'{str(ai_family).replace("AddressFamily.", "")}\n'
            f'IP Address: {ai_addr[0]}'
        )


def print_ips_with_gethostbyname(host_name: str):
    print(f'Getting name for "{host_name}"...\n' f'Using gethostbyname() function.')
    official_name, alias_list, address_list = socket.gethostbyname_ex(host_name)

    print(f'Official name: "{official_name}"')

    for alias in alias_list:
        print(f'Alias: "{alias}"')

    for addr in address_list:
        print(f'IP Address: "{addr}"')


if '__main__' == __name__:
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <hostname>')
        sys.exit(os.EX_USAGE)

    print_ips_with_getaddrinfo(sys.argv[1])
    time.sleep(1)
    print()
    print_ips_with_gethostbyname(sys.argv[1])

    sys.exit(os.EX_OK)
