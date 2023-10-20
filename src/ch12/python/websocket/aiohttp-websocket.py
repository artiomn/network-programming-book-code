#!/usr/bin/env python3

import asyncio
import aiohttp
from aiohttp import web


async def websocket_handler(request):
    ws = web.WebSocketResponse()
    await ws.prepare(request)

    async for msg in ws:
        if aiohttp.WSMsgType.TEXT == msg.type:
            if 'close' == msg.data:
                await ws.close()
            else:
                await ws.send_str(f'{msg.data}/answer')
        elif aiohttp.WSMsgType.ERROR == msg.type:
            print(f'ws connection closed with exception {ws.exception()}')

    print('websocket connection closed')

    return ws


async def main():
    server = web.Server(websocket_handler)
    runner = web.ServerRunner(server)
    await runner.setup()
    site = web.TCPSite(runner, 'localhost', 12345)
    await site.start()

    # pause here for very long time by serving HTTP requests and
    # waiting for keyboard interruption
    await asyncio.sleep(10 * 3600)


asyncio.run(main())
