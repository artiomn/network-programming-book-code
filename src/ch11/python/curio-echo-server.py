#!/usr/bin/env python

from curio import run, tcp_server


async def echo_handler(client, addr):
    print(f'Connection from {addr}')
    while True:
        data = await client.recv(1000)
        print(f'Data: {data}')
        if not data:
            break
        await client.sendall(data)
    print('Connection closed')


if '__main__' == __name__:
    run(tcp_server, '', 12345, echo_handler)
