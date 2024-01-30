#!/usr/bin/env python3

import argparse
import asyncio
import sys
from dataclasses import dataclass, asdict
import json
from time import time, ctime
from typing import Tuple, Optional, Callable, List
from concurrent.futures import ThreadPoolExecutor


@dataclass(frozen=True, repr=True)
class ChatMessage:
    sender_id: str
    receiver_id: str
    send_time: float
    text: str


class ChatProtocol(asyncio.Protocol):
    def __init__(self, on_message, on_disconnect):
        print('New protocol instance was created...')
        self._on_message = on_message
        self._on_disconnect = on_disconnect
        self.transport: Optional[asyncio.transports.Transport] = None
        self._peer_name = None

    def connection_made(self, transport):
        self._peer_name = transport.get_extra_info('peername')
        print(f'Connection peer name: {self._peer_name}')

        self.transport = transport

    def data_received(self, data):
        print(f'Data received from [{self._peer_name}]: {data!r}')
        message = json.loads(data.decode())

        if self._on_message is not None:
            self._on_message(ChatMessage(**message), self.transport)

    def connection_lost(self, exc):
        print(f'Connection lost, exc: {exc}')
        self._disconnect()

    def eof_received(self):
        print('Close the client socket')
        self._disconnect()

    def _disconnect(self):
        try:
            if self._on_disconnect is not None:
                self._on_disconnect(self)
        finally:
            self.transport.close()


class Communicator:
    """Communication layer."""

    def __init__(
        self,
        client_addr: Optional[Tuple[str, int]] = None,
        server_addr: Optional[Tuple[str, int]] = None,
        on_message: Optional[Callable[[ChatMessage, asyncio.transports.Transport], None]] = None,
    ):
        print(f'Client = {client_addr}, server = {server_addr}')
        self._server_addr = (str(server_addr[0]), int(server_addr[1])) if server_addr is not None else None
        self._client_addr = (str(client_addr[0]), int(client_addr[1])) if client_addr is not None else None
        self._on_message = on_message
        self._server: Optional[asyncio.Server] = None
        self._client: Optional[Tuple[asyncio.transports.Transport, ChatProtocol]] = None
        self._protos: List[ChatProtocol] = []

    @property
    def on_message(self):
        return self._on_message

    @on_message.setter
    def on_message(self, on_message):
        self._on_message = on_message

    async def run(self):
        print('Starting communication layer...')
        if self._server_addr is not None:
            print(f'Starting server [{self._server_addr[0]}:{self._server_addr[1]}]...')
            self._server = await asyncio.get_running_loop().create_server(
                self._proto_factory, host=self._server_addr[0], port=self._server_addr[1], reuse_address=True
            )
            print(f'Server started: {self._server}')

        if self._client_addr is not None:
            print(f'Starting client [{self._client_addr[0]}:{self._client_addr[1]}]...')
            self._client = await asyncio.get_running_loop().create_connection(
                self._proto_factory, host=self._client_addr[0], port=self._client_addr[1]
            )
            print(f'Client started: {self._client[0]}, {self._client[1]}')
        print('Communication layer started.')

    def stop(self):
        if self._server is not None:
            self._server.close()
        if self._client is not None:
            self._client[0].close()

    def send_message(self, msg: ChatMessage, exceptions: Optional[List[Tuple[str, int]]] = None):
        print(f'Sending message {msg}...')

        if exceptions is None:
            exceptions = []

        print(f'Sockets count = {len(self._protos)}')
        for s in self._protos:
            try:
                peer_name = s.transport.get_extra_info('peername')
                if peer_name in exceptions:
                    continue

                print(f'Sending from {s.transport.get_extra_info("sockname")} to ' f'{peer_name}')
                s.transport.write(self._marshall_message(msg))
            except BaseException as e:
                print(e)

    @staticmethod
    def _marshall_message(msg: ChatMessage) -> bytes:
        return json.dumps(asdict(msg)).encode()

    def _proto_factory(self):
        proto = ChatProtocol(self._on_message, self._on_disconnect)
        self._protos.append(proto)

        return proto

    def _on_disconnect(self, proto: ChatProtocol):
        print(f'{proto} was closed.')

        for i, p in enumerate(self._protos):
            if p == proto:
                del self._protos[i]
                return


class MessengerLogic:
    """Messenger business logic."""

    def __init__(self, my_nickname: str, communicator: Communicator):
        self._my_nickname = my_nickname
        self._comm = communicator
        self._comm.on_message = self._on_message
        self._running = False

        self._user_io_pool_exc = None

    async def run_forever(self):
        if self._running:
            return

        self._running = True

        self._user_io_pool_exc = ThreadPoolExecutor()

        await self._comm.run()

        try:
            while self._running:
                text = await asyncio.get_running_loop().run_in_executor(self._user_io_pool_exc, self._get_text)
                try:
                    receiver_nickname, text = (t.strip() for t in text.split(',', 1))
                    self._comm.send_message(ChatMessage(self._my_nickname, receiver_nickname, time(), text))
                except ValueError:
                    print('Type message in the format "receiver, text".')

                await asyncio.sleep(0.1)
        finally:
            self.stop()

    def stop(self):
        if not self._running:
            return

        self._running = False
        self._user_io_pool_exc.shutdown()
        self._comm.stop()

    def _on_message(self, msg: ChatMessage, transport: asyncio.transports.Transport):
        if msg.receiver_id == self._my_nickname:
            print(f'[{ctime(msg.send_time)}] MESSAGE FOR ME FROM "{msg.sender_id}": {msg.text}')
            return

        print('Resending msg...')
        self._comm.send_message(msg, [transport.get_extra_info('peername'), transport.get_extra_info('sockname')])
        print('Resend completed.')

    @staticmethod
    def _get_text() -> str:
        return input('Enter message in the format "receiver, text" > ')


async def main():
    parser = argparse.ArgumentParser(prog=__file__, description='P2P messenger example')
    parser.add_argument(
        '--server',
        action='store',
        type=str,
        nargs='?',
        default='0.0.0.0:5423',
        help='Server address, where others can be connected',
    )
    parser.add_argument(
        '--client',
        action='store',
        type=str,
        nargs='?',
        default='127.0.0.1:5423',
        help='Address of the server to connect',
    )
    parser.add_argument('--nickname', type=str, required=True)
    # parser.add_argument('--cert', type=argparse.FileType('r'))

    args = parser.parse_args()

    if args.server is None and args.client is None:
        raise SystemExit('Server, client or both must be specified!')

    comm_layer = Communicator(
        client_addr=args.client.split(':') if args.client is not None and args.client.lower() != 'no' else None,
        server_addr=args.server.split(':') if args.server is not None and args.server.lower() != 'no' else None,
    )

    return await MessengerLogic(args.nickname, comm_layer).run_forever()


try:
    bl = asyncio.run(main())
except KeyboardInterrupt:
    print('Exiting...')
    sys.exit()
