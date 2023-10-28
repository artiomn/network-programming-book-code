#!/usr/bin/env python3

import asyncio
from websockets.server import serve


async def echo(websocket):
    async for message in websocket:
        print(f'Received: {message}')
        await websocket.send(message)


async def main():
    async with serve(echo, 'localhost', 12345):
        await asyncio.Future()  # run forever


asyncio.run(main())
