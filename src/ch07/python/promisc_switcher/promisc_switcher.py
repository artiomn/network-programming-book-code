#!/usr/bin/env python3

import ctypes
from fcntl import ioctl
import socket
import sys
from typing import Optional


class InterfacePromiscSwitcher(ctypes.Structure):
    IFF_PROMISC = 0x100
    SIOCGIFFLAGS = 0x8913
    SIOCSIFFLAGS = 0x8914

    _fields_ = [('ifr_ifrn', ctypes.c_char * 16), ('ifr_flags', ctypes.c_short)]

    def __init__(self, if_name: str, *args, **kw):
        super().__init__(*args, **kw)
        self._sock: socket.socket
        if 'socket' in kw:
            self._sock = kw['socket']
        else:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.ifr_ifrn = if_name.encode()

    @property
    def socket(self) -> Optional[socket.socket]:
        return self._sock

    def set_promisc(self):
        sock = self._sock
        if sys.platform == 'win32':
            sock.bind((self.ifr_ifrn, 0))
            sock.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)
        else:
            ioctl(sock.fileno(), self.SIOCGIFFLAGS, self)
            self.ifr_flags |= self.IFF_PROMISC  # pylint: disable=E1101(no-member)
            ioctl(sock.fileno(), self.SIOCSIFFLAGS, self)

    def unset_promisc(self, close_socket: bool = True):
        if sys.platform == 'win32':
            self._sock.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)
        else:
            self.ifr_flags &= ~self.IFF_PROMISC  # pylint: disable=E1101(no-member)
            ioctl(self._sock.fileno(), self.SIOCSIFFLAGS, self)
        if close_socket:
            self._sock.close()

    def __enter__(self):
        self.set_promisc()
        return self

    def __exit__(self, *args):
        self.unset_promisc()


if '__main__' == __name__:
    from os import popen
    from sys import argv

    if len(argv) != 2:
        print(f'{argv[0]} <interface name>')
        sys.exit(1)

    interface_name = argv[1]

    with InterfacePromiscSwitcher(interface_name) as ifreq:
        print(f'Promiscuous mode for the "{interface_name}" enabled:')
        print(popen(f'ip a show {interface_name}').readline())  # nosec B605

    print(f'Promiscuous mode for the "{interface_name}" disabled:')
    print(popen(f'ip a show {interface_name}').readline())  # nosec B605
