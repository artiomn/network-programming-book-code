#!/usr/bin/env python3

from PyQt6.QtNetwork import QTcpServer, QTcpSocket
from PyQt6.QtCore import qDebug


class TcpServer:
    """Qt TCP server."""

    def __init__(self, parent=None, port: int | None = None):
        self._tcp_server = QTcpServer(parent)
        self._client_socket: QTcpSocket | None = None
        self._tcp_server.newConnection.connect(self._new_connection)

        port = port if port is not None else 0
        if self._tcp_server.listen(port=port):
            qDebug(f'Server started on port {port}')
        else:
            qDebug(f'Server could not start: {self._tcp_server.errorString()}')
            raise OSError(self._tcp_server.errorString())

    def _new_connection(self):
        self._client_socket = client_socket = self._tcp_server.nextPendingConnection()
        pa = f'{client_socket.peerAddress().toString()}: {client_socket.peerPort()}'
        qDebug(f'New connection from {pa}')

        try:
            client_socket.write(b'PyQt TCP server\r\n')
            client_socket.flush()
            client_socket.readyRead.connect(self._connection_handler)
            client_socket.waitForReadyRead(10000)
        finally:
            qDebug(f'Close connection {pa}')
            self._client_socket.close()
            self._client_socket = None

    def _connection_handler(self):
        qDebug('Client sent data:' f'{bytearray(self._client_socket.readLine()).decode()}')

    def wait_for_connection(self, msecs=0):
        self._tcp_server.waitForNewConnection(msecs)


srv = TcpServer(port=12345)

while True:
    srv.wait_for_connection(10)
