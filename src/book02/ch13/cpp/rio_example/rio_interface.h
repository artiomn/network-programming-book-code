#pragma once

#include <array>
#include <memory>
#include <vector>

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

extern "C"
{
#include <MSWSock.h>
#include <memoryapi.h>
}


class RIO
{
public:
    static constexpr ULONG rio_pending_recvs = 256;
    static constexpr ULONG rio_max_results = 32;
    // 64KB for TCP channel.
    static constexpr DWORD buffer_size = 65536;

public:
    using RIOBuf = std::unique_ptr<char, void(*)(char*)>;
    struct EXTENDED_RIO_BUF : public RIO_BUF
    {
        DWORD operation_id_;
    };

public:
    static SOCKET make_socket(int domain, int type, int proto);

public:
    RIO(const SOCKET rio_sock, void *completion_key = (void *)1) : rio_(init_rio(rio_sock)),
        completion_key_(completion_key), buffer_(make_buffers())
    {
        make_iocp();
    }

    RIO(const RIO&) = delete;
    RIO& operator=(const RIO&) = delete;
    RIO(RIO&&) = delete;
    RIO& operator=(RIO&&) = delete;

    ~RIO();

    const RIO_EXTENSION_FUNCTION_TABLE &rio() const { return rio_; }
    HANDLE get_iocp() const { return iocp_handle_; }
    RIO_CQ &get_cq() { return completion_queue_; }
    RIO_RQ &get_rq() { return request_queue_; }
    auto get_buffer_id() const { return buffer_id_; }
    EXTENDED_RIO_BUF* get_send_buf(DWORD id) { return &send_bufs_[id]; }

public:
    char* init_data_exchange(SOCKET client_sock);
    int notify() { return rio_.RIONotify(completion_queue_); }
    ULONG read_completion_queue(std::array<RIORESULT, rio_max_results> &results);

    void recv(PRIO_BUF p_data, ULONG data_buffer_count, DWORD flags, PVOID request_context = nullptr);
    void send(PRIO_BUF p_data, ULONG data_buffer_count, DWORD flags, PVOID request_context = nullptr);

protected:
    static RIO_EXTENSION_FUNCTION_TABLE init_rio(const SOCKET rio_sock);
    void make_client_queues(SOCKET client_sock);
    RIOBuf make_buffers();
    void init_recv_bufs();
    void make_iocp();

private:
    const RIO_EXTENSION_FUNCTION_TABLE rio_;
    const void *completion_key_;

    HANDLE iocp_handle_ = INVALID_HANDLE_VALUE;
    RIO_BUFFERID buffer_id_ = RIO_INVALID_BUFFERID;
    RIOBuf buffer_;

    OVERLAPPED overlapped_ = {};
    RIO_NOTIFICATION_COMPLETION completion_type_;
    RIO_CQ completion_queue_ = RIO_INVALID_CQ;
    RIO_RQ request_queue_ = RIO_INVALID_RQ;
    std::array<EXTENDED_RIO_BUF, rio_pending_recvs> recv_bufs_;
    std::array<EXTENDED_RIO_BUF, rio_pending_recvs> send_bufs_;
};