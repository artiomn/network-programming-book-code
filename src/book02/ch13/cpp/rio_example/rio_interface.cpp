#include "rio_interface.h"

#include <system_error>


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
    CloseHandle(iocp_handle_);
    rio_.RIODeregisterBuffer(buffer_id_);
}


RIO_CQ RIO::make_client_queues(SOCKET client_sock)
{
    RIO_CQ queue = rio_.RIOCreateCompletionQueue(rio_pending_recvs * 4, &completion_type_);
    if (RIO_INVALID_CQ == queue)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIOCreateCompletionQueue()");
    }

    request_queue_ =
        rio_.RIOCreateRequestQueue(client_sock, rio_pending_recvs, 1, rio_pending_recvs, 1, queue, queue, nullptr);
    if (RIO_INVALID_RQ == request_queue_)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIOCreateRequestQueue()");
    }

    return queue;
}


RIO::RIOBuf RIO::make_buffers()
{
    const DWORD total_buffer_size = buffer_size * rio_pending_recvs * 2;
    UCHAR cur_numa_node_number;

    if (!GetNumaProcessorNode(GetCurrentProcessorNumber(), &cur_numa_node_number))
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "GetNumaProcessorNode()");
    }
    char* raw_buffer =
        reinterpret_cast<char*>(VirtualAlloc(nullptr, total_buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    buffer_id_ = rio_.RIORegisterBuffer(raw_buffer, total_buffer_size);
    if (buffer_id_ == RIO_INVALID_BUFFERID)
    {
        VirtualFree(raw_buffer, 0, MEM_RELEASE);
        throw std::system_error(WSAGetLastError(), std::system_category(), "RIORegisterBuffer()");
    }

    return std::unique_ptr<char, void (*)(char *)>(raw_buffer, [](char *p) { VirtualFree(p, 0, MEM_RELEASE); });
}


ULONG RIO::read_completion_queue(RIO_CQ &queue, std::array<RIORESULT, rio_max_results> &results)
{
    auto result = rio_.RIODequeueCompletion(queue, results.data(), rio_max_results);

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
                throw std::system_error(err, std::system_category(), "Initial RIOReceive() failed at index " + std::to_string(i));
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

    if (0 != WSAIoctl(rio_sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &function_table_id, sizeof(GUID),
                        reinterpret_cast<void**>(&result), sizeof(result), &bytes, nullptr, nullptr))
    {
        throw std::system_error(GetLastError(), std::system_category(), "WSAIoctl() with SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER");
    }
    return result;
}


void RIO::make_iocp()
{
    iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    completion_type_ =
    {
        .Type = RIO_IOCP_COMPLETION,
        .Iocp =
        {
            .IocpHandle = iocp_handle_,
            .CompletionKey = const_cast<void*>(completion_key_),
            .Overlapped = &overlapped_
        }
    };
}


std::tuple<RIO_CQ, RIO::RIOBuf> RIO::init_data_exchange(SOCKET client_sock)
{
    auto result = std::make_tuple(make_client_queues(client_sock), make_buffers());

    init_recv_bufs();

    return result;
}