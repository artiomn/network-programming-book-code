#!/usr/bin/env python3

from tuntap import TunTap


tun = TunTap(nic_type='Tun', nic_name='tun0')
print(tun.name, tun.ip, tun.mask)

tap = TunTap(nic_type='Tap', nic_name='tap0')

tap.config(ip='192.168.1.10', mask='255.255.255.0', gateway='192.168.1.254')
print(tap.mac)

buf = tun.read(10)
tun.write(buf)

tap.close()
tun.close()
