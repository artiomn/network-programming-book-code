#include <glib.h>
#include <gio/gio.h>


/* this function will get called everytime a client attempts to connect */
gboolean
incoming_callback(GSocketService *service,
                  GSocketConnection *connection,
                  GObject *source_object,
                  gpointer user_data)
{
    g_print("Received Connection from client!\n");
    GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM (connection));
    gchar message[1024];
    g_input_stream_read(istream,
                        message,
                        1024,
                        NULL,
                        NULL);
    g_print("Message was: \"%s\"\n", message);
    return FALSE;
}


int main(int argc, const char *argv[])
{
    /* initialize glib */
    g_type_init();

    GError *error = NULL;

    /* create the new socketservice */
    GSocketService *service = g_socket_service_new ();

    /* connect to the port */
    g_socket_listener_add_inet_port ((GSocketListener*)service,
                                    1500, /* your port goes here */
                                    NULL,
                                    &error);

    /* don't forget to check for errors */
    if (error != NULL)
    {
        g_error(error->message);
    }

    /* listen to the 'incoming' signal */
    g_signal_connect(service,
                     "incoming",
                     G_CALLBACK (incoming_callback),
                     NULL);

    /* start the socket service */
    g_socket_service_start(service);

    /* enter mainloop */
    g_print("Waiting for client!\n");
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return EXIT_SUCCESS;
}

