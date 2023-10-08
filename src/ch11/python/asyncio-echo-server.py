#!/usr/bin/env python

import asyncio


async def handle_echo(reader, writer):
    while True:
        data = await reader.read(100)

        if not data.strip():
            break

        message = data.decode()
        addr = writer.get_extra_info('peername')

        print(f'Received {message!r} from {addr!r}')

        print(f'Send: {message!r}')
        writer.write(data)
        await writer.drain()

    print('Close the connection')
    writer.close()
    await writer.wait_closed()


async def main():
    server = await asyncio.start_server(handle_echo, '127.0.0.1', 12345)

    addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
    print(f'Serving on {addrs}')

    async with server:
        await server.serve_forever()


asyncio.run(main())
