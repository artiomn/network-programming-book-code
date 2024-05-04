#!/usr/bin/env python3

import asyncio
from pathlib import Path
import capnp

capnp.remove_import_hook()
echo_capnp = capnp.load(str((Path(__file__).parent / '../../capnp' / 'echo.capnp').resolve()))


async def main():
    host, port = 'localhost', 12345
    client = capnp.TwoPartyClient(await capnp.AsyncIoStream.create_connection(host=host, port=port))

    # Bootstrap the Echo interface
    echo_client = client.bootstrap().cast_as(echo_capnp.Echo)

    request_promise = echo_client.echo('Test request')

    print(f'Awaiting response for the {request_promise}...')

    response = await request_promise

    print(f'Server response: "{response.echo_reply}"')


async def run_main():
    await main()


asyncio.run(capnp.run(run_main()))
