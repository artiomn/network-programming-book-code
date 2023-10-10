#!/usr/bin/env python

from gevent.server import StreamServer


# This handler will be run for each incoming connection in a dedicated greenlet.
def echo(socket, address):
    print(f'New connection from {address}')

    # Using a makefile because we want to use readline().
    file_obj = socket.makefile(mode='rb')

    while True:
        line = file_obj.readline()
        if not line:
            print('Client disconnected')
            break

        print(f'Data: {line!r}')
        socket.sendall(line)

    file_obj.close()


if '__main__' == __name__:
    server = StreamServer(('127.0.0.1', 12345), echo)

    print('Starting echo server')
    # To start the server asynchronously, use its start() method.
    # We use blocking serve_forever() here because we have no other jobs
    server.serve_forever()
