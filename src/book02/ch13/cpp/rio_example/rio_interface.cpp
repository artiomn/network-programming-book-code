#include "rio_interface.h"

#include <system_error>


DWORD round_up(DWORD value, DWORD alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}


SOCKET RIO::make_socket(int domain, int type, int proto)
{
    if (auto sock = WSASocket(domain, type, proto, nullptr, 0, WSA_FLAG_REGISTERED_IO); INVALID_SOCKET != sock)
    {
        return sock;
    }
    throw std::system_error(WSAGetLastError(), std::system_category(), "WSASocket()");
}


RIO::~RIO()
{
    if (buffer_id_ != RIO_INVALID_BUFFERID)
    {
        rio_.RIODeregisterBuffer(buffer_id_);
    }

    if (completion_queue_ != RIO_INVALID_CQ)
    {
        rio_.RIOCloseCompletionQueue(completion_queue_);
    }

    if (iocp_handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(iocp_handle_);
    }

    buffer_.reset();
}


void RIO::make_client_queues(SOCKET client_sock)
{
    completion_queue_ = rio_.RIOCreateCompletionQueue(rio_pending_recvs * 4, &completion_type_);
    if (RIO_INVALID_CQ == completion_queue_)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIOCreateCompletionQueue()");
    }

    request_queue_ = rio_.RIOCreateRequestQueue(
        client_sock, rio_pending_recvs, 1, rio_pending_recvs, 1, completion_queue_, completion_queue_, nullptr);
    if (RIO_INVALID_RQ == request_queue_)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIOCreateRequestQueue()");
    }
}


RIO::RIOBuf RIO::make_buffers()
{
    SYSTEM_INFO systemInfo = {};
    GetSystemInfo(&systemInfo);

    const DWORD total_buffer_size = round_up(buffer_size * rio_pending_recvs * 2, systemInfo.dwAllocationGranularity);

    UCHAR cur_numa_node_number;

    if (!GetNumaProcessorNode(GetCurrentProcessorNumber(), &cur_numa_node_number))
    {
        throw std::system_error(GetLastError(), std::system_category(), "GetNumaProcessorNode()");
    }

    RIO::RIOBuf buffer{
        static_cast<char *>(VirtualAllocExNuma(
            GetCurrentProcess(), nullptr, total_buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE,
            cur_numa_node_number)),
        [](char *p) { VirtualFreeEx(GetCurrentProcess(), p, 0, MEM_RELEASE); }};

    buffer_id_ = rio_.RIORegisterBuffer(buffer.get(), total_buffer_size);
    if (RIO_INVALID_BUFFERID == buffer_id_)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIORegisterBuffer()");
    }

    return buffer;
}


ULONG RIO::read_completion_queue(std::array<RIORESULT, rio_max_results> &results)
{
    auto result = rio_.RIODequeueCompletion(completion_queue_, results.data(), rio_max_results);

    if (RIO_CORRUPT_CQ == result)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIO queue was corrupted");
    }

    return result;
}


void RIO::init_recv_bufs()
{
    DWORD offset = 0;
    for (DWORD i = 0; i < rio_pending_recvs; ++i)
    {
        recv_bufs_[i].operation_id_ = i;
        recv_bufs_[i].BufferId = buffer_id_;
        recv_bufs_[i].Offset = offset;
        recv_bufs_[i].Length = buffer_size;
        offset += buffer_size;

        if (!rio_.RIOReceive(request_queue_, &recv_bufs_[i], 1, 0, &recv_bufs_[i]))
        {
            DWORD err = WSAGetLastError();
            if (err != WSA_IO_PENDING)
            {
                throw std::system_error(
                    err, std::system_category(), "Initial RIOReceive() failed at index " + std::to_string(i));
            }
        }
    }

    for (DWORD i = 0; i < rio_pending_recvs; ++i)
    {
        send_bufs_[i].operation_id_ = i;
        send_bufs_[i].BufferId = buffer_id_;
        send_bufs_[i].Offset = offset;
        send_bufs_[i].Length = buffer_size;
        offset += buffer_size;
    }
}


void RIO::recv(PRIO_BUF p_data, ULONG data_buffer_count, DWORD flags, PVOID request_context)
{
    if (!rio_.RIOReceive(request_queue_, p_data, data_buffer_count, flags, request_context))
    {
        if (DWORD err = WSAGetLastError(); err != WSA_IO_PENDING)
        {
            throw std::system_error(err, std::system_category(), "RIOReceive()");
        }
    }
}


void RIO::send(PRIO_BUF p_data, ULONG data_buffer_count, DWORD flags, PVOID request_context)
{
    if (!rio_.RIOSend(request_queue_, p_data, data_buffer_count, flags, request_context))
    {
        if (DWORD err = WSAGetLastError(); err != WSA_IO_PENDING)
        {
            throw std::system_error(err, std::system_category(), "RIOSend()");
        }
    }
}


RIO_EXTENSION_FUNCTION_TABLE RIO::init_rio(const SOCKET rio_sock)
{
    GUID function_table_id = WSAID_MULTIPLE_RIO;
    DWORD bytes = 0;
    RIO_EXTENSION_FUNCTION_TABLE result;

    if (0 != WSAIoctl(
                 rio_sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &function_table_id, sizeof(GUID),
                 reinterpret_cast<void **>(&result), sizeof(result), &bytes, nullptr, nullptr))
    {
        throw std::system_error(
            WSAGetLastError(), std::system_category(), "WSAIoctl() with SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER");
    }
    return result;
}


void RIO::make_iocp()
{
    iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    completion_type_ = {
        .Type = RIO_IOCP_COMPLETION,
        .Iocp = {
            .IocpHandle = iocp_handle_,
            .CompletionKey = const_cast<void *>(completion_key_),
            .Overlapped = &overlapped_}};
}


char *RIO::init_data_exchange(SOCKET client_sock)
{
    make_client_queues(client_sock);
    init_recv_bufs();

    return buffer_.get();
}
