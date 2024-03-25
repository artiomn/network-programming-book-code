#!/usr/bin/env python3

import sys
from win32.win32api import GetLastError
from win32.win32file import ReadFile, WriteFile, CloseHandle, FlushFileBuffers
from win32.win32pipe import (
    ConnectNamedPipe,
    CreateNamedPipe,
    DisconnectNamedPipe,
    NMPWAIT_USE_DEFAULT_WAIT,
    PIPE_ACCESS_DUPLEX,
    PIPE_READMODE_MESSAGE,
    PIPE_TYPE_MESSAGE,
    PIPE_UNLIMITED_INSTANCES,
    PIPE_WAIT,
    SetNamedPipeHandleState,
)
from winerror import ERROR_MORE_DATA, ERROR_PIPE_BUSY


h_pipe = None
message = 'I\'m server: '
pipe_name = R'\\.\pipe\test_pipe'
buf_size = 1024

# Try to open a named pipe; wait for it, if necessary.
while True:
    # read_buffer
    h_pipe = CreateNamedPipe(
        pipe_name,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        len(message) * 2,
        buf_size,
        NMPWAIT_USE_DEFAULT_WAIT,
        None,
    )

    if h_pipe:
        break

    # Exit if an error other than ERROR_PIPE_BUSY occurs.
    if GetLastError() != ERROR_PIPE_BUSY:
        print('Could not open pipe.')
        sys.exit(1)

try:
    while True:
        print('Waiting for connections...')
        try:
            ConnectNamedPipe(h_pipe, None)
        except OSError:
            print('Could not connect pipe')
            sys.exit(1)

        print('Pipe connected, reading data...')

        # The pipe connected; change to message-read mode.
        try:
            SetNamedPipeHandleState(h_pipe, PIPE_READMODE_MESSAGE, None, None)
        except BaseException:
            print('SetNamedPipeHandleState() failed')
            sys.exit(1)

        # Read from the pipe if there is more data in the message.
        # Exit if an error other than ERROR_MORE_DATA occurs.
        result, data = ReadFile(h_pipe, buf_size)
        if result and GetLastError() != ERROR_MORE_DATA:
            DisconnectNamedPipe(h_pipe)
            sys.exit(1)
        else:
            print(f'Data read: {data}')
            new_msg = f'{message}{data.decode()}'.encode()
            print('Sending data...')

            err_code, written_bytes = WriteFile(h_pipe, new_msg)

            if not err_code and written_bytes:
                print(f'Bytes sent: {written_bytes}')
            else:
                print('Failed to send data.')
                sys.exit(1)

            if FlushFileBuffers(h_pipe):
                print('FlushFileBuffers() failed.')
                sys.exit(1)

            if DisconnectNamedPipe(h_pipe):
                print('Could not disconnect pipe.')
                sys.exit(1)
finally:
    CloseHandle(h_pipe)
