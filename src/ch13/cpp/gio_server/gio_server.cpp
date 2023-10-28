extern "C"
{
#include <gio/gio.h>
#include <glib.h>
}

#include <iostream>
#include <string>


gboolean incoming_callback(
    GSocketService *service, GSocketConnection *connection, GObject *source_object, gpointer user_data)
{
    g_print("Received Connection from client!\n");
    GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    gchar message[1024];
    auto read_size = g_input_stream_read(istream, message, sizeof(message), nullptr, nullptr);
    std::cout << "Message was: " << std::string(message, message + read_size) << std::endl;
    return FALSE;
}


int main(int argc, const char *argv[])
{
    GError *error = nullptr;

    GSocketService *service = g_socket_service_new();

    g_socket_listener_add_inet_port(
        reinterpret_cast<GSocketListener *>(service),
        // Порт.
        1500, nullptr, &error);

    if (error != nullptr)
    {
        g_error(error->message);
    }

    g_signal_connect(service, "incoming", G_CALLBACK(incoming_callback), nullptr);

    g_socket_service_start(service);

    g_print("Waiting for a client!\n");
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    return EXIT_SUCCESS;
}
