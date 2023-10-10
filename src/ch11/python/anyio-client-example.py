#!/usr/bin/env python

from anyio import connect_tcp, run


async def main():
    async with await connect_tcp('google.com', 80) as client:
        await client.send(b'GET / HTTP/1.1\r\nHost: google.com\r\n\r\n')
        response = await client.receive()
        print(response[:30])


run(main, backend='trio')
