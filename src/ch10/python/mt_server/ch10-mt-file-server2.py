#!/usr/bin/env python3

import socket
import sys

# pylint: disable=C0412(ungrouped-imports)
from os import path
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from threading import current_thread

try:
    from os import pathconf

    MAX_PATH = pathconf('/', 'PC_PATH_MAX')
except ImportError:
    MAX_PATH = 250


BUFFER_SIZE = 4096


class Client:
    def __init__(self, client_sock: socket.socket):
        print(f'Client [{client_sock.fileno()}] was created...')
        self._sock_file = client_sock.makefile('rwb')

    def recv_file_path(self) -> Path | None:
        print('Reading user request...')
        request_data_bin = self._sock_file.readline()
        if not request_data_bin:
            return None

        request_data = request_data_bin.decode().rstrip()
        print(f'Request = "{request_data}"')

        file_path = path.normpath(Path(request_data))

        cwd = str(Path.cwd())
        if str(file_path).find(cwd) == 0:
            file_path = request_data[len(cwd) :]

        return Path(cwd) / file_path.lstrip('/\\')

    def send_file(self, file_path: Path):
        if not (file_path and (file_path.exists() and file_path.is_file())):
            return False

        print(f'Sending file "{file_path}"...')

        try:
            with file_path.open('rb') as f:
                while data := f.read(BUFFER_SIZE):
                    self._sock_file.write(data)

        except BaseException as e:
            print(e)
            return False

        return True


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
        print(f'Request completed with a result = {task.result()}...\n' 'Removing from list.')
        return False

    return True


if '__main__' == __name__:
    if len(sys.argv) < 2:
        print('Usage: file-server2.py <port>')
        sys.exit(1)
    port = sys.argv[1]

    address_a = socket.getaddrinfo(
        host=None,
        port=port,
        family=socket.AF_INET,
        type=socket.SOCK_STREAM,
        proto=socket.IPPROTO_TCP,
        flags=socket.AI_PASSIVE,
    )

    address = address_a[0][-1]
    pending_tasks = []

    with ThreadPoolExecutor(max_workers=10) as thread_pool, socket.create_server(address, reuse_port=True) as server:
        print(f'Listening on port {port}...\n' f'Server path: {Path.cwd()}')

        while True:
            sock, client_addr = server.accept()
            print(f'Client from {client_addr}...')
            pending_tasks.append(thread_pool.submit(send_file_to_the_client, sock))

            pending_tasks[:] = [task for task in pending_tasks if need_to_remove_task(task)]
