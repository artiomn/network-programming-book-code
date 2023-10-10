#!/usr/bin/env python

import trio


async def producer(send_channel):
    async with send_channel:
        for i in range(3):
            await send_channel.send(f'message {i}')


async def consumer(receive_channel):
    async with receive_channel:
        async for value in receive_channel:
            print(f'Got value {value!r}')


async def main():
    send_channel, receive_channel = trio.open_memory_channel(0)
    async with trio.open_nursery() as nursery:
        nursery.start_soon(producer, send_channel)
        nursery.start_soon(consumer, receive_channel)


trio.run(main)
