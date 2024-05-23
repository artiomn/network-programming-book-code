#!/usr/bin/env python3

import sys
from pyroute2 import IPRoute


def print_if_attrs(if_attrs):
    for attr_name, attr_val in if_attrs:
        if 'IFLA_OPERSTATE' == attr_name:
            print(f'Oper state: {attr_val}')
        elif 'IFLA_ADDRESS' == attr_name:
            print(f'Address: {attr_val}')


ipr = IPRoute()

# lookup the index
devs = ipr.link_lookup(ifname=sys.argv[1])

if not devs:
    print(f'Device "{sys.argv[1]}" was not found!')
    sys.exit(1)

dev_index = devs[0]

print(f'Devices count = {len(devs)}, Device index = {dev_index}')

print_if_attrs(ipr.link('get', index=dev_index)[0]['attrs'])

# Bring it down.
ipr.link('set', index=dev_index, state='down')

# Change the interface MAC address and rename it just for fun.
ipr.link('set', index=dev_index, address='00:11:22:33:44:55', ifname='eno2')

# Add primary IP address.
ipr.addr('add', index=dev_index, address='10.0.0.1', mask=24, broadcast='10.0.0.255')

# Bring it up.
ipr.link('set', index=dev_index, state='up')

print_if_attrs(ipr.link('get', index=dev_index)[0]['attrs'])
