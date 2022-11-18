#!/usr/bin/env python3

import socket
import sys

from os import path, pathconf
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from threading import current_thread
from typing import Optional


BUFFER_SIZE = 4096
MAX_PATH = pathconf('/', 'PC_PATH_MAX')


class Transceiver:
    def __init__(self, client_sock: socket.socket):
        self._client_sock = client_sock

    @property
    def socket(self):
        return self._client_sock

    def send_buffer(self, buffer):
        transmit_bytes_count = 0

        while transmit_bytes_count != len(buffer):
            result = self._client_sock.send(buffer[transmit_bytes_count:],
                                            len(buffer) - transmit_bytes_count)
            transmit_bytes_count += result

    def send_file(self, file_path: Path):
        try:
            with file_path.open('rb') as f:
                print(f'Sending file "{file_path}"...')
                data = f.read(BUFFER_SIZE)
                while data:
                    self.send_buffer(data)
                    data = f.read(BUFFER_SIZE)

        except BaseException as e:
            print(e)
            return False

        return True

    def get_request(self) -> Optional[str]:
        print('Reading user request...')
        path_buffer = bytearray(MAX_PATH)
        buffer = memoryview(path_buffer)
        recv_bytes = 0

        while True:
            result = self._client_sock.recv_into(buffer[recv_bytes:], len(buffer) - recv_bytes)

            if 0 == result:
                break

            pos = [pos for pos, char in enumerate(buffer[recv_bytes:recv_bytes + result])
                   if char in (ord('\n'), ord('\r'))]

            if pos:
                recv_bytes += pos[0]
                break

            recv_bytes += result

        result = path_buffer[:recv_bytes].decode()
        print(f'Request = "{result}"')

        return result


class Client:
    def __init__(self, client_sock: socket.socket):
        print(f'Client [{client_sock.fileno()}] was created...')
        self._tsr = Transceiver(client_sock)

    def recv_file_path(self):
        request_data = self._tsr.get_request()
        if not request_data:
            return

        file_path = path.normpath(Path(request_data))

        cwd = str(Path.cwd())
        if str(file_path).find(cwd) == 0:
            file_path = request_data[len(cwd):]

        return Path(cwd) / file_path.lstrip('/\\')

    def send_file(self, file_path: Path):
        if not (file_path.exists() and file_path.is_file()):
            return False

        return self._tsr.send_file(file_path)


def send_file_to_the_client(c_sock: socket.socket):
    try:
        print(f'Client tid = {current_thread().name} [{current_thread().ident}]')
        client = Client(c_sock)

        file_path = client.recv_file_path()

        print(f'Trying to send "{file_path}"...')

        result = client.send_file(file_path)

        if result:
            print('File was sent')

        else:
            print('File sending error!')
    except BaseException as e:
        print(e)
        result = False
    c_sock.close()
    print(f'Client with tid = {current_thread().name} [{current_thread().native_id}] exiting...')
    return result


def need_to_remove_task(task):
    if not task.running():
        print(f'Request completed with a result = {task.result()}...\n'
              'Removing from list.')
        return False

    return True


if '__main__' == __name__:
    if len(sys.argv) < 2:
        print('Usage: file-server1.py <port>')
        sys.exit(1)
    port = sys.argv[1]

    address = socket.getaddrinfo(host=None, port=port,
                                 family=socket.AF_INET, type=socket.SOCK_STREAM, proto=socket.IPPROTO_TCP,
                                 flags=socket.AI_PASSIVE)

    address = address[0][-1]
    pending_tasks = []

    with ThreadPoolExecutor(max_workers=10) as thread_pool,\
            socket.create_server(address, reuse_port=True) as server:
        print(f'Listening on port {port}...\n'
              f'Server path: {Path.cwd()}')

        while True:
            sock, client_addr = server.accept()
            print(f'Client from {client_addr}...')
            pending_tasks.append(thread_pool.submit(send_file_to_the_client, sock))

            pending_tasks[:] = [task for task in pending_tasks if need_to_remove_task(task)]
