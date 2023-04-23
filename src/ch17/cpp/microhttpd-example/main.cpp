/**
 * @file minimal_example.c
 * @brief minimal example for how to use libmicrohttpd
 * @author Christian Grothoff
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <microhttpd.h>
}


#define PAGE "<html><head><title>libmicrohttpd demo</title></head><body>libmicrohttpd demo</body></html>"

static enum MHD_Result ahc_echo(
    void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size, void **req_cls)
{
    static int aptr;
    const char *me = static_cast<const char *>(cls);
    struct MHD_Response *response;
    enum MHD_Result ret;

    (void)url;              /* Unused. Silent compiler warning. */
    (void)version;          /* Unused. Silent compiler warning. */
    (void)upload_data;      /* Unused. Silent compiler warning. */
    (void)upload_data_size; /* Unused. Silent compiler warning. */

    if (0 != strcmp(method, "GET")) return MHD_NO;

    if (&aptr != *req_cls)
    {
        // Do never respond on first call.
        *req_cls = &aptr;
        return MHD_YES;
    }

    // Сброс по завершении.
    *req_cls = NULL;
    response = MHD_create_response_from_buffer(strlen(me), (void *)me, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}


int main(int argc, const char *const *argv)
{
    struct MHD_Daemon *d;

    if (argc != 2)
    {
        printf("%s PORT\n", argv[0]);
        return 1;
    }

    d = MHD_start_daemon(
        /* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
        MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
        /* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG |
           MHD_USE_POLL, */
        /* MHD_USE_THREAD_PER_CONNECTION |
           MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG |
           MHD_USE_POLL, */
        /* MHD_USE_THREAD_PER_CONNECTION |
           MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
        atoi(argv[1]), NULL, NULL, &ahc_echo, const_cast<char *>(PAGE), MHD_OPTION_CONNECTION_TIMEOUT,
        (unsigned int)120, MHD_OPTION_STRICT_FOR_CLIENT, (int)1, MHD_OPTION_END);

    if (d == NULL) return 1;

    getc(stdin);
    MHD_stop_daemon(d);

    return 0;
}
