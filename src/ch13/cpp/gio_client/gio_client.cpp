#include <string>

extern "C"
{
#include <glib.h>
#include <gio/gio.h>
}


int main (int argc, const char *argv[])
{
    GError *error = nullptr;

    GSocketConnection *connection = nullptr;
    GSocketClient *client = g_socket_client_new();

    connection = g_socket_client_connect_to_host(client,
                                           "localhost",
                                           1500,
                                           nullptr,
                                           &error);

    if (error != nullptr)
    {
        g_error (error->message);
    }
    else
    {
        g_print("Connection successful!\n");
    }

    GInputStream *istream = g_io_stream_get_input_stream(
        G_IO_STREAM(connection));
    GOutputStream *ostream = g_io_stream_get_output_stream(
        G_IO_STREAM(connection));

    const std::string msg = "Hello server!\n";

    g_output_stream_write(ostream,
                          msg.c_str(),
                          msg.size(),
                          nullptr,
                          &error);

    if (error != nullptr)
    {
        g_error(error->message);
    }

    return EXIT_SUCCESS;
}
