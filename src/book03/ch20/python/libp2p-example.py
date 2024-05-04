#!/usr/bin/env python3

import asyncio
from libp2p import new_node
from libp2p.tcp import TCP
from libp2p.noise import Noise
from libp2p.mplex import MPLEX


async def main():
    transport = TCP()
    security = Noise()
    muxer = MPLEX()

    node = await new_node(transport=transport, security=security, muxer=muxer)

    print(f'Node is running: {node}!')


asyncio.run(main())
