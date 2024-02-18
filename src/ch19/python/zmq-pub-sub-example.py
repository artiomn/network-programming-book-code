#!/usr/bin/python3

import random
import time
import sys

import zmq


def main():
    # Context and sockets.
    ctx = zmq.Context()
    publisher = ctx.socket(zmq.XPUB)
    publisher.bind('inproc://12345')

    # Prepare our context and subscriber
    subscriber = ctx.socket(zmq.SUB)
    subscriber.setsockopt(zmq.SUBSCRIBE, b'')
    subscriber.connect('inproc://12345')

    random.seed(time.time())
    while True:
        rand = random.randint(1, 100)
        publisher.send(f'{sys.argv[1]}-{rand}'.encode())
        print(f'I: sending message {rand}')

        if subscriber.poll(1000) & zmq.POLLIN:
            message = subscriber.recv(zmq.NOBLOCK)
            print(f'I: received message "{message}"')
        time.sleep(1)


if '__main__' == __name__:
    main()
