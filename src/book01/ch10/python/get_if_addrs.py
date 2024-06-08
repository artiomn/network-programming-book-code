#!/usr/bin/env python3

import ctypes
import ctypes.util
import socket
from ctypes import (
    POINTER,
    Structure,
    Union,
    c_byte,
    c_char_p,
    c_uint,
    c_uint16,
    c_uint32,
    c_ushort,
    c_void_p,
    cast,
    get_errno,
    pointer,
)


class sockaddr(Structure):
    _fields_ = [('sa_family', c_ushort), ('sa_data', c_byte * 14)]


class sockaddr_in(Structure):
    _fields_ = [('sin_family', c_ushort), ('sin_port', c_uint16), ('sin_addr', c_byte * 4)]


class sockaddr_in6(Structure):
    _fields_ = [
        ('sin6_family', c_ushort),
        ('sin6_port', c_uint16),
        ('sin6_flowinfo', c_uint32),
        ('sin6_addr', c_byte * 16),
        ('sin6_scope_id', c_uint32),
    ]


class ifa_ifu(Union):
    _fields_ = [('ifu_broadaddr', POINTER(sockaddr)), ('ifu_dstaddr', POINTER(sockaddr))]


class ifaddrs(Structure):
    pass


# pylint: disable=protected-access
ifaddrs._fields_ = [
    ('ifa_next', POINTER(ifaddrs)),
    ('ifa_name', c_char_p),
    ('ifa_flags', c_uint),
    ('ifa_addr', POINTER(sockaddr)),
    ('ifa_netmask', POINTER(sockaddr)),
    ('ifa_ifu', ifa_ifu),
    ('ifa_data', c_void_p),
]

LibC = ctypes.CDLL(ctypes.util.find_library('c'))
freeifaddrs = LibC.freeifaddrs
getifaddrs = LibC.getifaddrs


class NetworkInterface:
    def __init__(self, name):
        self.name = name.decode() if isinstance(name, bytes) else name
        self.index = socket.if_nametoindex(name)
        self.addresses = {}
        self.netmask = {}

    def __str__(self):
        return (
            f'{self.name}:\n    index = {self.index}, '
            f'\n    addr IPv4 = {self.addresses.get(socket.AF_INET)}'
            f'\n    netmask IPv4 = {self.netmask.get(socket.AF_INET)}'
            f'\n    addr IPv6 = {self.addresses.get(socket.AF_INET6)}'
            f'\n    netmask IPv6 = {self.netmask.get(socket.AF_INET6)}'
        )


class NetworkInterfaces:
    def __init__(self):
        self._ifa_pointer = POINTER(ifaddrs)()

    def __del__(self):
        del self._ifa_pointer

    def _ifa_iter(self):
        p = self._ifa_pointer
        while p:
            yield p.contents
            p = p.contents.ifa_next

    @staticmethod
    def _get_address(sa):
        try:
            sa = sa.contents
            family = sa.sa_family
            addr = None
            if socket.AF_INET == family:
                sa = cast(pointer(sa), POINTER(sockaddr_in)).contents
                addr = socket.inet_ntop(family, sa.sin_addr)
            elif socket.AF_INET6 == family:
                sa = cast(pointer(sa), POINTER(sockaddr_in6)).contents
                addr = socket.inet_ntop(family, sa.sin6_addr)
        except ValueError:
            # NULL pointer access.
            family, addr = None, None
        return family, addr

    def _get_network_interfaces(self):
        result = getifaddrs(pointer(self._ifa_pointer))

        if result != 0:
            raise OSError(get_errno())

        try:
            retval = []
            for ifa in self._ifa_iter():
                i_face = NetworkInterface(ifa.ifa_name)
                retval.append(i_face)
                addr_family, addr = NetworkInterfaces._get_address(ifa.ifa_addr)
                nm_family, netmask = NetworkInterfaces._get_address(ifa.ifa_netmask)

                if addr:
                    i_face.addresses[addr_family] = addr
                if netmask:
                    i_face.netmask[nm_family] = netmask

            return retval
        finally:
            freeifaddrs(self._ifa_pointer)

    @property
    def interfaces(self):
        return self._get_network_interfaces()


if __name__ == '__main__':
    for ni in NetworkInterfaces().interfaces:
        print(f'{str(ni)}\n')
