#!/usr/bin/env python

import asyncio


class EchoServerProtocol(asyncio.Protocol):
    def connection_made(self, transport):
        peername = transport.get_extra_info('peername')
        print(f'Connection from {peername}')
        self.transport = transport

    def data_received(self, data):
        message = data.decode()
        print(f'Data received: {message!r}')

        print(f'Send: {message!r}')
        self.transport.write(data)

    def connection_lost(self, exc):
        print(f'Connection lost, exc: {exc}')
        self.transport.close()

    def eof_received(self):
        print('Close the client socket')
        self.transport.close()


async def main():
    # Get a reference to the event loop as we plan to use
    # low-level APIs.
    loop = asyncio.get_running_loop()

    # Lambda is necessary to recreate the protocol objects.
    # pylint: disable=unnecessary-lambda
    server = await loop.create_server(lambda: EchoServerProtocol(), '127.0.0.1', 12345)

    async with server:
        await server.serve_forever()


asyncio.run(main())
