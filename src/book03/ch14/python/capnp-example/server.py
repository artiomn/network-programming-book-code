#!/usr/bin/env python3

from pathlib import Path
import sys

import asyncio
import capnp

sys.path.append(str((Path(__file__).parent / '../../capnp').resolve()))

# pylint: disable=C0413(wrong-import-position)
import echo_capnp  # noqa


class ExampleEchoServerImpl(echo_capnp.Echo.Server):
    async def echo(self, echo_request, _context):
        print(f'Client request: {echo_request}')
        return f'from server = {echo_request}'


async def new_connection(stream):
    await capnp.TwoPartyServer(stream, bootstrap=ExampleEchoServerImpl()).on_disconnect()


async def main():
    host, port = '0.0.0.0', 12345
    server = await capnp.AsyncIoStream.create_server(new_connection, host, port)
    async with server:
        await server.serve_forever()


if '__main__' == __name__:
    asyncio.run(capnp.run(main()))
