#!/usr/bin/env python3

import socketserver
import sys


class UpperEchoTCPHandler(socketserver.StreamRequestHandler):
    """Request handler class."""

    def handle(self):
        self.data = self.rfile.readline().strip()

        # Direct reading via request:
        # self.request.recv(1024).strip()

        print(f'{self.client_address[0]} wrote:')
        print(self.data)

        self.wfile.write(self.data.upper())

        # Direct sending via request:
        # self.request.sendall(self.data.upper())


if len(sys.argv) < 2:
    print('server <port>')
    sys.exit(1)

port = int(sys.argv[1])

with socketserver.TCPServer(('localhost', port), UpperEchoTCPHandler) as server:
    print(f'Server starting on port {port}')
    server.serve_forever()
