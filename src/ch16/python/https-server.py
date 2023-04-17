#!/usr/bin/python3

import ssl
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path

cur_dir = Path(__file__).parent

https_server = HTTPServer(('localhost', 1443), BaseHTTPRequestHandler)

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain(cur_dir / 'server.pem', cur_dir / 'key.pem')

https_server.socket = context.wrap_socket(https_server.socket, server_side=True)

https_server.serve_forever()
