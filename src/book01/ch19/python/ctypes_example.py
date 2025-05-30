import ctypes

# mypy: disable-error-code="attr-defined"
from ctypes import windll
from ctypes.wintypes import DWORD, USHORT
import ipaddress
from socket import htonl


def GetIpAddrTable():
    size = DWORD()
    # First call to receive the correct dwSize back.
    windll.iphlpapi.GetIpAddrTable(None, ctypes.byref(size), 0)

    class MIB_IPADDRROW(ctypes.Structure):
        _fields_ = [
            ('dwAddr', DWORD),
            ('dwIndex', DWORD),
            ('dwMask', DWORD),
            ('dwBCastAddr', DWORD),
            ('dwReasmSize', DWORD),
            ('unused1', USHORT),
            ('wType', USHORT),
        ]

    class MIB_IPADDRTABLE(ctypes.Structure):
        _fields_ = [('dwNumEntries', DWORD), ('table', MIB_IPADDRROW * size.value)]

    ip_table = MIB_IPADDRTABLE()
    if (rc := windll.iphlpapi.GetIpAddrTable(ctypes.byref(ip_table), ctypes.byref(size), 0)) != 0:
        raise OSError(f'GetIpAddrTable returned {rc}')

    return ip_table


addrs = GetIpAddrTable()

for i in range(addrs.dwNumEntries):
    print(ipaddress.ip_address(htonl(addrs.table[i].dwAddr)))
