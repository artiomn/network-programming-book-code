#!/usr/bin/env python3

import gi

gi.require_version('Gtk', '3.0')

# pylint: disable=C0413(wrong-import-position)
from gi.repository import Gio, GLib, Gtk  # noqa: E402


cancelable = Gio.Cancellable()


def cb_read(pipe, results, *args):
    request = pipe.read_bytes_finish(results)
    request = request.get_data()
    connection = args[:][0]
    out_stream = Gio.IOStream.get_output_stream(connection)

    if request:
        print(f'Client request = {request.decode().strip()}')
        out_stream.write(request)
    else:
        return

    if not cancelable.is_cancelled():
        # pipe.read_bytes_async(256, 0, cancelable, cb_read, connection)
        GLib.idle_add(pipe.read_bytes_async, 256, 0, None, cb_read, connection)


try:
    # pylint: disable=W0613(unused-argument)
    def incoming_callback(service, connection, source_object, user_data=None):
        ostream = Gio.IOStream.get_output_stream(connection)
        ostream.write(b'Hello from the server!\n')
        istream = Gio.IOStream.get_input_stream(connection)
        istream.read_bytes_async(256, 0, cancelable, cb_read, connection)

        return False

    gio_service = Gio.SocketService.new()
    Gio.SocketListener.add_inet_port(gio_service, 12345)
    gio_service.connect('incoming', incoming_callback)
    gio_service.start()
    Gtk.main()

except gi.repository.GLib.GError as err:
    print(err)
