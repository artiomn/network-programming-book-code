#!/usr/bin/env python3

import asyncio
from sniffio import current_async_library, current_async_library_cvar
import sniffio
import trio


async def print_library():
    print(f'This is: {current_async_library()} - {current_async_library_cvar.get()}')


try:
    print(current_async_library())
except sniffio.AsyncLibraryNotFoundError:
    print(f'Library was not detected: {current_async_library_cvar.get()}!')

trio.run(print_library)
asyncio.run(print_library())
