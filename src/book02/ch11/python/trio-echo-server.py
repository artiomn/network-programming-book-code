#!/usr/bin/env python3

import trio


async def echo_server(server_stream):
    print('Connected')

    try:
        async for data in server_stream:
            print(f'echo server received data {data!r}')
            await server_stream.send_all(data)

        print('Connection closed')

    except Exception as exc:
        # Unhandled exceptions will propagate into our parent and take
        # down the whole program. If the exception is KeyboardInterrupt,
        # that's what we want, but otherwise maybe not...
        print(f'{exc!r}')


async def main():
    await trio.serve_tcp(echo_server, 12345)


# We could also just write 'trio.run(trio.serve_tcp, echo_server, PORT)', but real
# programs almost always end up doing other stuff too and then we'd have to go
# back and factor it out into a separate function anyway. So it's simplest to
# just make it a standalone function from the beginning.

trio.run(main)
