#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>

extern "C"
{
#include <uv.h>
}


uv_loop_t *loop;
struct sockaddr_in addr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle;

    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}


void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        std::cerr
            << "Write error " <<  uv_strerror(status)
            << std::endl;
    }

    delete req;
}


void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            std::cerr
                << "Read error " <<  uv_err_name(nread)
                << std::endl;
            uv_close(reinterpret_cast<uv_handle_t*>(client), nullptr);
        }
    }
    else if (nread > 0)
    {
        auto req = std::make_unique<uv_write_t>();
        uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
        uv_write(req.release(), client, &wrbuf, 1, echo_write);
    }

    if (buf->base)
    {
        delete[] buf->base;
    }
}


void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        std::cerr
            << "New connection error "
            << uv_strerror(status) << std::endl;
        return;
    }

    auto client = new uv_tcp_t;
    uv_tcp_init(loop, client);

    if (0 == uv_accept(server, reinterpret_cast<uv_stream_t*>(client)))
    {
        uv_read_start(reinterpret_cast<uv_stream_t*>(client), alloc_buffer, echo_read);
    }
    else
    {
        uv_close(reinterpret_cast<uv_handle_t*>(client), nullptr);
    }
}


int main()
{
    unsigned short port = 8192;

    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", port, &addr);

    uv_tcp_bind(&server, reinterpret_cast<const struct sockaddr*>(&addr), 0);
    int r = uv_listen(reinterpret_cast<uv_stream_t*>(&server), 128, on_new_connection);
    if (r)
    {
        std::cerr << "Listen error " << uv_strerror(r) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listening started on the port " << port << std::endl;

    return uv_run(loop, UV_RUN_DEFAULT);
}

