#!/usr/bin/env python3

import sys
from pathlib import Path

from PyQt6 import QtNetwork
from PyQt6.QtWidgets import QApplication


app = QApplication(sys.argv)
if len(app.arguments()) != 2:
    print(f'{Path(__file__).name} <host[:port]>')
    sys.exit(1)

host = app.arguments()[1]
try:
    host, port = host.split(':')
except ValueError:
    port = 12345

sock = QtNetwork.QTcpSocket()

sock.connectToHost(host, int(port))

if sock.waitForConnected():
    sock.write(b'Test!')

    sock.readyRead.connect(lambda: (print(sock.readLine()), app.quit()))
else:
    raise ConnectionError(sock.errorString())

app.exec()
