#!/usr/bin/env python3

import gi

gi.require_version('Gtk', '3.0')

# pylint: disable=C0413(wrong-import-position)
from gi.repository import Gio, GLib, Gtk  # noqa: E402


cancelable = Gio.Cancellable()


def cb_read(pipe, results, *args):
    response = pipe.read_bytes_finish(results)
    response = response.get_data().decode().strip()

    if response:
        print(f'Client response = {response}')

    if not cancelable.is_cancelled():
        GLib.idle_add(pipe.read_bytes_async, 256, 1, None, cb_read, ())

    cancelable.cancel()


try:
    # pylint: disable=W0613(unused-argument)
    def incoming_callback(service, connection, source_object, user_data=None):
        ostream = Gio.IOStream.get_output_stream(connection)
        ostream.write(b'Hello from the server!\n')
        istream = Gio.IOStream.get_input_stream(connection)
        istream.read_bytes_async(256, 0, cancelable, cb_read)

        return False

    gio_service = Gio.SocketService.new()
    Gio.SocketListener.add_inet_port(gio_service, 1500)
    gio_service.connect('incoming', incoming_callback)
    gio_service.start()
    Gtk.main()

except gi.repository.GLib.GError as err:
    print(err)
