#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#if defined(_WIN32)
#    include <process.h>
#    ifndef getpid
#        define getpid _getpid
#    endif
#else
#    include <unistd.h>
#endif

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>


using namespace std::chrono_literals;

// Define the Packet Constants.
// ping packet size.
constexpr size_t ping_packet_size = 64;
constexpr auto ping_sleep_rate = 1000000us;

// Gives the timeout delay for receiving packets.
constexpr auto recv_timeout = 1s;

// Echo Request.
constexpr int ICMP_ECHO = 8;

// Echo Response.
constexpr int ICMP_ECHO_REPLY = 0;


#pragma pack(push, 1)
// Windows doesn't have this structure.
struct icmphdr
{
    uint8_t type; /* message type */
    uint8_t code; /* type sub-code */
    uint16_t checksum;
    union
    {
        struct
        {
            uint16_t id;
            uint16_t sequence;
        } echo; /* echo datagram */
        // cppcheck-suppress unusedStructMember
        uint32_t gateway; /* gateway address */
        struct
        {
            // cppcheck-suppress unusedStructMember
            uint16_t __unused;
            // cppcheck-suppress unusedStructMember
            uint16_t mtu;
            // cppcheck-suppress unusedStructMember
        } frag; /* path mtu discovery */
    } un;
};


//
// IPv4 Header (without any IP options)
//

typedef struct ip_hdr
{
    unsigned char ip_verlen;    // 4-bit IPv4 version.
                                // 4-bit header length (in 32-bit words).
    unsigned char ip_tos;       // IP type of service.
    uint16_t ip_totallength;    // Total length.
    uint16_t ip_id;             // Unique identifier.
    uint16_t ip_offset;         // Fragment offset field.
    unsigned char ip_ttl;       // Time to live.
    unsigned char ip_protocol;  // Protocol(TCP, UDP, etc.).
    uint16_t ip_checksum;       // IP checksum.
    uint32_t ip_srcaddr;        // Source address.
    uint32_t ip_destaddr;       // Destination address.
} IPV4_HDR, *PIPV4_HDR;
#pragma pack(pop)


//
// Ping packet class.
//

class PingPacket
{
public:
    using BufferType = std::vector<char>;

public:
    // Zero packet constructor. Need for recv.
    explicit PingPacket(size_t packet_size = ping_packet_size) : data_buffer_(packet_size, 0)
    {
        assert(data_buffer_.size() >= sizeof(icmphdr));
    }

    explicit PingPacket(uint16_t packet_id, uint16_t packet_sequence_number, size_t packet_size = ping_packet_size)
    {
        create_new_packet(packet_id, packet_sequence_number, packet_size);
    }

    explicit PingPacket(BufferType &&packet_buffer) : data_buffer_{packet_buffer}
    {
        assert(data_buffer_.size() >= sizeof(icmphdr));
    }

    explicit PingPacket(const BufferType::const_iterator &start, const BufferType::const_iterator &end)
        : data_buffer_{start, end}
    {
        assert(data_buffer_.size() >= sizeof(icmphdr));
    }

    const icmphdr &header() const { return *get_header_from_buffer(); }

    size_t size() const { return data_buffer_.size(); }

    uint16_t checksum() const
    {
        const uint16_t *buf = reinterpret_cast<const uint16_t *>(data_buffer_.data());
        size_t len = data_buffer_.size();
        uint32_t sum = 0;

        for (sum = 0; len > 1; len -= 2) sum += *buf++;
        if (1 == len) sum += *reinterpret_cast<const uint8_t *>(buf);
        sum = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);

        uint16_t result = sum;

        return ~result;
    }

public:
    operator BufferType() const { return data_buffer_; }

    operator const BufferType::value_type *() const { return data_buffer_.data(); }

    operator BufferType::value_type *() { return data_buffer_.data(); }

    operator bool() const
    {
        auto real_header = const_cast<icmphdr *>(get_header_from_buffer());
        auto old_checksum = real_header->checksum;

        // Checksum field in the packet must be equal to 0 before checksum calculation.
        real_header->checksum = 0;
        auto new_checksum = checksum();
        real_header->checksum = old_checksum;

        return old_checksum == new_checksum;
    }

private:
    icmphdr *get_header_from_buffer() const
    {
        assert(!data_buffer_.empty());
        return reinterpret_cast<icmphdr *>(const_cast<BufferType::value_type *>(data_buffer_.data()));
    }

    void create_new_packet(uint16_t packet_id, uint16_t packet_sequence_number, size_t packet_size)
    {
        static_assert(1 == sizeof(BufferType::value_type));
        assert(packet_size > sizeof(icmphdr));

        data_buffer_.resize(packet_size);

        auto p_header = get_header_from_buffer();

        p_header->type = ICMP_ECHO;
        p_header->code = 0;
        p_header->checksum = 0;
        p_header->un.echo.id = htons(packet_id);
        p_header->un.echo.sequence = htons(packet_sequence_number);

        std::generate(
            std::next(data_buffer_.begin(), sizeof(icmphdr)), data_buffer_.end(),
            [i = 'a']() mutable { return i <= 'z' ? i++ : i = 'a'; });

        get_header_from_buffer()->checksum = checksum();
    }

private:
    BufferType data_buffer_;
};


template <class PClass>
class PacketFactory
{
public:
    typedef PClass PacketClass;

    static constexpr uint16_t max_id = 2 ^ (8 * sizeof(uint16_t)) - 1;

public:
    PacketFactory() : pid_(getpid()), sequence_number_{0} {}

public:
    PacketClass create_request() { return PacketClass(pid_, sequence_number_++); }

    PacketClass create_response() { return PacketClass(); }

    PacketClass create_response(
        const typename PacketClass::BufferType::iterator start, const typename PacketClass::BufferType::iterator end)
    {
        return PacketClass(start, end);
    }

    PacketClass create_response(typename PacketClass::BufferType &&buffer) { return PacketClass(std::move(buffer)); }

private:
    int pid_{0};
    uint16_t sequence_number_{0};
};


inline void send_ping(
    const socket_wrapper::Socket &sock, const std::string &hostname, const sockaddr_in &host_address,
    bool ip_headers_enabled = true)
{
    using namespace std::chrono;

    socket_wrapper::SocketWrapper sock_wrap;

#if !defined(WIN32)
    timeval tv = {
        .tv_sec = std::chrono::duration<int64_t>(duration_cast<seconds>(recv_timeout)).count(),
        // Ugly, but it will work.
        .tv_usec =
            (int64_t)(duration_cast<microseconds>(recv_timeout) - microseconds(duration_cast<seconds>(recv_timeout)))
                .count()};
#else
    auto tv = duration_cast<milliseconds>(recv_timeout).count();
#endif

    sockaddr_in r_addr;

    PacketFactory<PingPacket> ping_factory;

    constexpr int ttl_val = 255;
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, reinterpret_cast<const char *>(&ttl_val), sizeof(ttl_val)) != 0)
    {
        throw std::system_error(sock_wrap.get_last_error_code(), std::system_category(), "TTL setting failed!");
    }

    // Setting timeout of recv setting.
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv)) != 0)
    {
        throw std::system_error(
            sock_wrap.get_last_error_code(), std::system_category(), "Recv timeout setting failed!");
    }

    std::cout << "TTL = " << ttl_val << "\n";

#if !defined(WIN32)
    std::cout << "Recv timeout seconds = " << tv.tv_sec << "\n"
              << "Recv timeout microseconds = " << tv.tv_usec << std::endl;
#else
    std::cout << "Recv timeout seconds = " << tv << " ms\n" << std::endl;
#endif

    // Send ICMP packet in an infinite loop
    while (true)
    {
        const auto request = ping_factory.create_request();
        const auto &request_echo_header = request.header().un.echo;

        std::cout << "Sending packet " << ntohs(request_echo_header.sequence) << " to \"" << hostname << "\" "
                  << "request with id = " << ntohs(request_echo_header.id) << std::endl;

        if (sendto(
                sock, static_cast<const char *>(request), request.size(), 0,
                reinterpret_cast<const sockaddr *>(&host_address), sizeof(host_address)) < request.size())
        {
            std::cerr << "Packet sending failed: \"" << sock_wrap.get_last_error_string() << "\"" << std::endl;
            continue;
        }

        // Receive packet
        socklen_t addr_len = sizeof(sockaddr);
        r_addr.sin_family = AF_INET;
        r_addr.sin_addr = host_address.sin_addr;

        const auto buf_size = ip_headers_enabled ? ping_packet_size + sizeof(ip_hdr) : ping_packet_size;
        std::vector<char> buffer(buf_size, 0);

        const auto start_time = std::chrono::steady_clock::now();

        if (recvfrom(sock, buffer.data(), buffer.size(), 0, reinterpret_cast<sockaddr *>(&r_addr), &addr_len) < 0)
        {
            std::cerr << "Packet receiving failed: \"" << sock_wrap.get_last_error_string() << "\"" << std::endl;
            continue;
        }

        const auto ip_header_len = [&buffer]()
        { return (reinterpret_cast<const ip_hdr *>(buffer.data())->ip_verlen & 0x0f) * sizeof(uint32_t); };
        auto response = ip_headers_enabled
                            ? ping_factory.create_response(buffer.begin() + ip_header_len(), buffer.end())
                            : ping_factory.create_response(std::move(buffer));

        if (!response)
        {
            std::cerr << "Bad response checksum!" << std::endl;
        }

        if ((ICMP_ECHO_REPLY == response.header().type) && (0 == response.header().code))
        {
            const auto end_time = std::chrono::steady_clock::now();
            const auto &response_echo_header = response.header().un.echo;
            std::cout << "Receiving packet " << ntohs(response_echo_header.sequence) << " from \"" << hostname << "\" "
                      << "response with id = " << ntohs(response_echo_header.id) << ", time = "
                      << std::round(std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                                        end_time - start_time)
                                        .count())
                      << "ms" << std::endl;
        }

        std::this_thread::sleep_for(ping_sleep_rate);
    }
}
