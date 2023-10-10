#!/usr/bin/env python

import anyio
from anyio import create_tcp_listener, run


async def handle(client):
    async with client:
        try:
            while True:
                data = await client.receive(1024)
                print(f'Data: {data!r}')
                await client.send(data)
        except anyio.EndOfStream:
            print(f'Client {client} closed')


async def main():
    listener = await create_tcp_listener(local_port=12345)
    await listener.serve(handle)


run(main)
