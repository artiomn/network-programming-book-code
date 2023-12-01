#!/usr/bin/env python3

import gi
from gi.repository import Gio


cancelable = Gio.Cancellable()


try:
    client = Gio.SocketClient.new()
    connection = client.connect_to_host('localhost', 12345, None)
    istream = Gio.IOStream.get_input_stream(connection)
    ostream = Gio.IOStream.get_output_stream(connection)
    msg = 'Hello, server!\n'
    ostream.write(msg.encode())
    data = istream.read_bytes(1024, cancelable)
    print(data.get_data().decode().strip())
except gi.repository.GLib.GError as err:
    print(err)
