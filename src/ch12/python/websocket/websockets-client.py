#!/usr/bin/env python

from websockets.sync.client import connect


def main():
    with connect('ws://localhost:12345') as websocket:
        websocket.send('Test')
        message = websocket.recv()
        print(f'Received: {message}')


main()
