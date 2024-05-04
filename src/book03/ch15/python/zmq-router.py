#!/usr/bin/python3

import random
import time
import sys

import zmq


def main():
    # context and sockets
    ctx = zmq.Context()
    collector = ctx.socket(zmq.ROUTER)
    collector.bind("inproc://5558")
    publisher_r = ctx.socket(zmq.XPUB)
    publisher_r.bind("inproc://5557")
    # 	p = zmq.proxy(collector, publisher)

    # Prepare our context and subscriber
    subscriber = ctx.socket(zmq.SUB)
    subscriber.setsockopt(zmq.SUBSCRIBE, b'')
    subscriber.connect('inproc://5557')
    publisher = ctx.socket(zmq.DEALER)

    publisher.setsockopt(zmq.IDENTITY, f'I AM {sys.argv[1]}'.encode())
    publisher.connect('inproc://5558')

    random.seed(time.time())
    while True:
        if collector.poll(100) & zmq.POLLIN:
            message = collector.recv(zmq.NOBLOCK)
            print(f'I: publishing update {message}')
            publisher_r.send(message, zmq.NOBLOCK)

        if subscriber.poll(1000) & zmq.POLLIN:
            message = subscriber.recv(zmq.NOBLOCK)
            print('I: received message', message)
        else:
            rand = random.randint(1, 100)
            publisher.send(f'{sys.argv[1]}-{rand}'.encode())
            print(f'I: sending message {rand}')


if '__main__' == __name__:
    main()
