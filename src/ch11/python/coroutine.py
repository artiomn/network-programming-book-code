#!/usr/bin/env python3

import asyncio


async def async_func():
    print('Starting ...')
    await asyncio.sleep(1)
    print('... Ready!')


async def main():
    task = asyncio.create_task(async_func())
    await task


asyncio.run(main())
