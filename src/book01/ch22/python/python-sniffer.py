#!/usr/bin/env python3

import struct
import binascii
import socket
from typing import Any, final, Final


import sys
from abc import abstractmethod

from promisc_switcher.promisc_switcher import InterfacePromiscSwitcher


ETH_P_IP = 0x0800
ETH_P_ALL = 0x0003
SO_BINDTODEVICE = 25
RECV_BUF_SIZE = 65535


class NetworkProtocolHeader:
    channel_protocols: Final[set[type]] = set()
    protocols: Final[dict[int, type]] = {}
    format_string = ''

    @final
    def __init_subclass__(cls):
        cls.header_size = struct.calcsize(cls.format_string)
        try:
            proto_code = cls.protocol_code
        except AttributeError:
            NetworkProtocolHeader.channel_protocols.add(cls)
        else:
            proto_class = NetworkProtocolHeader.protocols.get(proto_code)

            if proto_class is not None:
                raise KeyError(f'Class for the protocol code {proto_code} already exists [{proto_class.__name__}]!')
            NetworkProtocolHeader.protocols[proto_code] = cls

    def __init__(self, packet):
        self._packet = packet

    @final
    @property
    def fields(self):
        return struct.unpack(self.format_string, self._packet[: self.header_size])

    @property
    @abstractmethod
    def inner_protocol_code(self) -> int:
        """Overridable protocol code."""

    @property
    @abstractmethod
    def payload_offset(self) -> int:
        """Overridable upper-protocol header offset."""

    @final
    @classmethod
    def unpack_packet(cls, base_protocol, packet, *, skip_channel_proto_check: bool = False) -> list[Any]:
        """Package unpacking method."""
        if not issubclass(base_protocol, NetworkProtocolHeader):
            raise TypeError(
                f'{base_protocol!r} is not a descendant class of the {cls.__class__.__name__}!',
            )
        if not skip_channel_proto_check and base_protocol not in cls.channel_protocols:
            raise KeyError(
                f'{base_protocol.__name__} is not a channel protocol class!',
            )

        headers = []

        cur_header = base_protocol(packet)
        headers.append(cur_header)

        while True:
            packet = packet[cur_header.payload_offset :]
            try:
                inner_code = cur_header.inner_protocol_code
            except NotImplementedError:
                break
            else:
                proto_header_class = cls.protocols.get(inner_code)
                if proto_header_class is None:
                    break
                cur_header = proto_header_class(packet)

                headers.append(cur_header)

        return headers


class EthernetHeader(NetworkProtocolHeader):
    # "!" - unpacking from the network byte order.
    format_string = '!6s6sH'

    def __init__(self, packet):
        super().__init__(packet)
        self.destination_mac_address, self.source_mac_address, self.protocol = self.fields
        self.header_size: int

    @property
    def inner_protocol_code(self) -> int:
        return int(self.protocol)

    @property
    def payload_offset(self) -> int:
        # pylint: disable=E1101(no-member)
        return self.header_size

    @staticmethod
    def print_mac(addr):
        pa = binascii.hexlify(addr).decode()
        return ':'.join([pa[i : i + 2] for i in range(0, len(pa), 2)])

    def __str__(self):
        return (
            f'[{self.__class__.__name__}]:\n'
            f'  Src MAC: {self.print_mac(self.destination_mac_address)}\n'
            f'  Dst MAC: {self.print_mac(self.source_mac_address)}\n'
            f'  EtherType: 0x{self.protocol:04x}'
        )


# pylint: disable=R0902(too-many-instance-attributes)
class Ipv4Header(NetworkProtocolHeader):
    protocol_code = 0x0800
    format_string = '!BBHHHBBH4s4s'
    word_length = 4

    def __init__(self, packet):
        super().__init__(packet)
        (
            self.ip_version,
            self.tos,
            self.total_length,
            self.identification,
            self.fragment_offset,
            self.ttl,
            self.protocol,
            self.header_checksum,
            self.source_address,
            self.destination_address,
        ) = self.fields
        self.ihl = self.ip_version & 0x0F
        self.ip_version &= 0xF0
        self.ip_version >>= 4

        self.header_size: int
        self.protocol = int(self.protocol)

        assert self.ip_version == 4
        assert self.ihl * self.word_length >= self.header_size  # pylint: disable=E1101(no-member)

    @property
    def inner_protocol_code(self) -> int:
        return int(self.protocol)

    @property
    def payload_offset(self) -> int:
        return int(self.ihl * self.word_length)

    def __str__(self):
        spacing = ' ' * 6
        return (
            f'    [{self.__class__.__name__}]:\n'
            f'{spacing}Payload Off: {self.payload_offset}\n'
            f'{spacing}TOS: {self.tos}\n'
            f'{spacing}TTL: {self.ttl}\n'
            f'{spacing}Proto: {self.protocol}\n'
            f'{spacing}Length: {self.total_length}\n'
            f'{spacing}Src IP: '
            f'{socket.inet_ntop(socket.AF_INET, self.source_address)}\n'
            f'{spacing}Dst IP: '
            f'{socket.inet_ntop(socket.AF_INET, self.destination_address)}'
        )


class Ipv6Header(NetworkProtocolHeader):
    # From the EtherType list.
    protocol_code = 0x86DD
    format_string = '!BBHHBB16s16s'

    word_length = 4

    def __init__(self, packet):
        super().__init__(packet)
        # pylint: disable=R0902(too-many-instance-attributes)
        (
            first_byte,
            second_byte,
            flow_label_lsb,
            self.payload_length,
            self.next_header,
            self.hop_limit,
            self.source_address,
            self.destination_address,
        ) = self.fields

        self.ip_version = (first_byte & 0xF0) >> 4
        self.traffic_class = ((first_byte & 0x0F) << 4) | ((second_byte & 0xF0) >> 4)
        self.flow_label = (second_byte & 0x0F << 16) | flow_label_lsb
        self.header_size: int

        assert self.ip_version == 6

    @property
    def inner_protocol_code(self) -> int:
        return int(self.next_header)

    @property
    def payload_offset(self) -> int:
        # pylint: disable=E1101(no-member)
        return int(self.header_size)

    def __str__(self):
        spacing = ' ' * 6
        printable_addr = socket.inet_ntop(socket.AF_INET6, self.destination_address)
        return (
            f'    [{self.__class__.__name__}]:\n'
            f'{spacing}Traffic Class: '
            f'{self.traffic_class}\n'
            f'{spacing}Flow Label: {self.flow_label}\n'
            f'{spacing}Next Header: {self.next_header}\n'
            f'{spacing}Payload Length: '
            f'{self.payload_length}\n'
            f'{spacing}Src IP: '
            f'{socket.inet_ntop(socket.AF_INET6, self.source_address)}\n'
            f'{spacing}Dst IP: '
            f'{spacing}{printable_addr}'
        )


class IcmpHeader(NetworkProtocolHeader):
    # From the IANA list.
    protocol_code = 1
    format_string = '!BBH'

    def __init__(self, packet):
        super().__init__(packet)
        self.message_type, self.code, self.checksum = self.fields
        self.header_size: int

    @property
    def inner_protocol_code(self) -> int:
        raise NotImplementedError('ICMP has not inner protocol!')

    @property
    def payload_offset(self) -> int:
        # pylint: disable=E1101(no-member)
        return int(self.header_size)

    def __str__(self):
        spacing = ' ' * 10
        return (
            f'        [{self.__class__.__name__}]:\n'
            f'{spacing}Message Type: {self.message_type}\n'
            f'{spacing}Code: {self.code}\n'
            f'{spacing}Checksum: 0x{self.checksum:04x}'
        )


class Icmp6Header(IcmpHeader):
    protocol_code = 58

    @property
    def inner_protocol_code(self) -> int:
        raise NotImplementedError('ICMP6 has not inner protocol!')


class TcpHeader(NetworkProtocolHeader):
    protocol_code = 6
    format_string = '!HHLLBBHHH'
    word_length = 4

    def __init__(self, packet):
        super().__init__(packet)
        (
            self.source_port,
            self.destination_port,
            self.sequence_number,
            self.acknowledge_number,
            offset_reserved,
            self.tcp_flag,
            self.window,
            self.checksum,
            self.urgent_pointer,
        ) = self.fields

        self.data_offset = ((offset_reserved & 0xF0) >> 4) * self.word_length

    @property
    def inner_protocol_code(self) -> int:
        raise NotImplementedError('TCP has arbitrary user-defined inner protocol!')

    @property
    def payload_offset(self) -> int:
        return int(self.data_offset)

    def __str__(self):
        spacing = ' ' * 10
        return (
            f'        [{self.__class__.__name__}]:\n'
            f'{spacing}Src Port: {self.source_port}\n'
            f'{spacing}Dst Port: {self.destination_port}\n'
            f'{spacing}Seq Number: {self.sequence_number}\n'
            f'{spacing}Ack Number: {self.acknowledge_number}\n'
            f'{spacing}Data Off: {self.data_offset}\n'
            f'{spacing}Flags: 0x{self.tcp_flag:04x}\n'
            f'{spacing}Window: {self.window}\n'
            f'{spacing}Urgent Ptr: {self.urgent_pointer}\n'
            f'{spacing}Checksum: 0x{self.checksum:04x}'
        )


class UdpHeader(NetworkProtocolHeader):
    # Номер из списка IANA.
    protocol_code = 17
    format_string = '!HHHH'

    def __init__(self, packet):
        super().__init__(packet)
        self.source_port, self.destination_port, self.length, self.checksum = self.fields
        self.header_size: int

    @property
    def inner_protocol_code(self) -> int:
        raise NotImplementedError('UDP has arbitrary user-defined inner protocol!')

    @property
    def payload_offset(self) -> int:
        # pylint: disable=E1101(no-member)
        return int(self.header_size)

    def __str__(self):
        spacing = ' ' * 10
        return (
            f'        [{self.__class__.__name__}]:\n'
            f'{spacing}Src Port: {self.source_port}\n'
            f'{spacing}Dst Port: {self.destination_port}\n'
            f'{spacing}Length: {self.length}\n'
            f'{spacing}Checksum: 0x{self.checksum:04x}'
        )


if len(sys.argv) != 2:
    print(f'{sys.argv[0]} <interface name>')
    sys.exit(1)

interface_name = sys.argv[1]

with InterfacePromiscSwitcher(interface_name) as ips:
    if 'win32' == sys.platform:
        sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
        sock.setsockopt(socket.SOL_SOCKET, SO_BINDTODEVICE, interface_name.encode())

        sock.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
    else:
        sock = socket.socket(socket.PF_PACKET, socket.SOCK_RAW, socket.ntohs(ETH_P_ALL))

    sock.bind((interface_name, 0))

    while True:
        data = sock.recvfrom(RECV_BUF_SIZE)
        for p in NetworkProtocolHeader.unpack_packet(EthernetHeader, data[0]):
            print(p)
        print()
