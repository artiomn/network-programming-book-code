#!/usr/bin/env python3

import asyncio


async def tcp_client(host, port):
    reader, writer = await asyncio.open_connection(host, port)

    writer.write(f'GET / HTTP/1.1\r\nHost: {host}\r\n\r\n'.encode())
    await writer.drain()

    data = await reader.read(100)
    print(f'Received: {data.decode()!r}')
    writer.close()
    await writer.wait_closed()


asyncio.run(tcp_client('ya.ru', 80))
