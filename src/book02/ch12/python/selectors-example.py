#!/usr/bin/python3

import selectors
import socket

selector = selectors.DefaultSelector()


def accept(srv_sock, _):
    conn, addr = srv_sock.accept()
    print(f'Accepted {addr}.')
    conn.setblocking(False)
    selector.register(conn, selectors.EVENT_READ, read)


def read(conn, _):
    data = conn.recv(1000)
    if data:
        print(f'Echoing {repr(data)} to {conn.getpeername()}...')
        # Synchronous.
        conn.send(data)
    else:
        selector.unregister(conn)
        conn.close()


sock = socket.socket()
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('localhost', 12345))
sock.listen(100)
sock.setblocking(False)
selector.register(sock, selectors.EVENT_READ, accept)

while True:
    events = selector.select()
    for key, mask in events:
        # Callback.
        key.data(key.fileobj, mask)
