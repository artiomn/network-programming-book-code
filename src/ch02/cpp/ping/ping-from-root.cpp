#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
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
const size_t ping_packet_size = 64;
const auto ping_sleep_rate = 1000000us;

// Gives the timeout delay for receiving packets.
const auto recv_timeout = 1s;

// Echo Request.
const int ICMP_ECHO = 8;

// Echo Response.
const int ICMP_ECHO_REPLY = 0;


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
    typedef std::vector<char> BufferType;

public:
    // Zero packet constructor. Need for recv.
    explicit PingPacket(size_t packet_size = ping_packet_size) : packet_size_{packet_size}
    {
        data_buffer_.resize(packet_size_);
    }

    PingPacket(uint16_t packet_id, uint16_t packet_sequence_number, size_t packet_size = ping_packet_size)
        : packet_size_{packet_size}
    {
        create_new_packet(packet_id, packet_sequence_number);
    }

    explicit PingPacket(BufferType &&packet_buffer) : packet_size_{packet_buffer.size()}, data_buffer_{packet_buffer}
    {
        assert(packet_size_ >= sizeof(icmphdr));
    }

    PingPacket(const BufferType::const_iterator &start, const BufferType::const_iterator &end)
        : packet_size_{static_cast<size_t>(std::distance(end, start))}, data_buffer_{start, end}
    {
        assert(packet_size_ >= sizeof(icmphdr));
    }

    const icmphdr &header() const { return *get_header_from_buffer(); }

    size_t size() { return data_buffer_.size(); }

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
        auto result = checksum();
        real_header->checksum = old_checksum;

        return old_checksum == result;
    }

private:
    icmphdr *get_header_from_buffer() const
    {
        return reinterpret_cast<icmphdr *>(const_cast<BufferType::value_type *>(data_buffer_.data()));
    }

    void create_new_packet(uint16_t packet_id, uint16_t packet_sequence_number)
    {
        static_assert(sizeof(BufferType::value_type) == 1);
        assert(packet_size_ > sizeof(icmphdr));

        data_buffer_.resize(packet_size_);

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
    const size_t packet_size_;
    BufferType data_buffer_;
};


template <class PClass>
class PacketFactory
{
public:
    typedef PClass PacketClass;

    const uint16_t max_id = 2 ^ (8 * sizeof(uint16_t)) - 1;

public:
    PacketFactory() : pid_(getpid()), sequence_number_{0} {}

public:
    PacketClass create_request() { return PacketClass(pid_, sequence_number_++); }

    PacketClass create_response() { return PingPacket(); }

    PacketClass create_response(
        const typename PacketClass::BufferType::iterator start, const typename PacketClass::BufferType::iterator end)
    {
        return PacketClass(start, end);
    }

    PacketClass create_response(typename PacketClass::BufferType &&buffer) { return PacketClass(std::move(buffer)); }

private:
    int pid_;
    uint16_t sequence_number_;
};


size_t ip_header_len(const PingPacket::BufferType &buffer)
{
    return (reinterpret_cast<const struct ip_hdr *>(buffer.data())->ip_verlen & 0x0f) * sizeof(uint32_t);
}


void send_ping(
    const socket_wrapper::Socket &sock, const std::string &hostname, const sockaddr_in &host_address,
    const bool ip_headers_enabled = true)
{
    int ttl_val = 255;
    using namespace std::chrono;

#if !defined(WIN32)
    struct timeval tv = {
        .tv_sec = std::chrono::duration<int64_t>(duration_cast<seconds>(recv_timeout)).count(),
        // Ugly, but it will works.
        .tv_usec =
            (int64_t)(duration_cast<microseconds>(recv_timeout) - microseconds(duration_cast<seconds>(recv_timeout)))
                .count()};
#else
    auto tv = duration_cast<milliseconds>(recv_timeout).count();
#endif

    struct sockaddr_in r_addr;

    PacketFactory<PingPacket> ping_factory;

    if (setsockopt(sock, IPPROTO_IP, IP_TTL, reinterpret_cast<const char *>(&ttl_val), sizeof(ttl_val)) != 0)
    {
        throw std::runtime_error("TTL setting failed!");
    }

    // Setting timeout of recv setting.
    /*    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) != 0)
        {
            throw std::runtime_error("Recv timeout setting failed!");
        }*/

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
        auto request = std::move(ping_factory.create_request());
        const auto &request_echo_header = request.header().un.echo;

        std::cout << "Sending packet " << ntohs(request_echo_header.sequence) << " to \"" << hostname << "\" "
                  << "request with id = " << ntohs(request_echo_header.id) << std::endl;

        if (sendto(
                sock, static_cast<const char *>(request), request.size(), 0,
                reinterpret_cast<const struct sockaddr *>(&host_address), sizeof(host_address)) < request.size())
        {
            std::cerr << "Packet sending failed: \""
                      << "\"" << std::endl;
            continue;
        }

        // Receive packet
        socklen_t addr_len = sizeof(sockaddr);
        r_addr.sin_family = AF_INET;
        r_addr.sin_addr = host_address.sin_addr;

        std::vector<char> buffer;
        const auto buf_size = ip_headers_enabled ? ping_packet_size + sizeof(ip_hdr) : ping_packet_size;

        buffer.resize(buf_size);
        auto start_time = std::chrono::steady_clock::now();

        if (recvfrom(sock, buffer.data(), buffer.size(), 0, reinterpret_cast<struct sockaddr *>(&r_addr), &addr_len) <
            buffer.size())
        {
            std::cerr << "Packet receiving failed: \""
                      << "\"" << std::endl;
            continue;
        }

        auto response = std::move(
            ip_headers_enabled ? ping_factory.create_response(buffer.begin() + ip_header_len(buffer), buffer.end())
                               : ping_factory.create_response(std::move(buffer)));

        if ((ICMP_ECHO_REPLY == response.header().type) && (0 == response.header().code))
        {
            auto end_time = std::chrono::steady_clock::now();
            const auto &response_echo_header = response.header().un.echo;
            std::cout << "Receiving packet " << ntohs(response_echo_header.sequence) << " from \"" << hostname << "\" "
                      << "response with id = " << ntohs(response_echo_header.id) << ", time = "
                      << std::round(
                             std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                                 end_time - start_time)
                                 .count() *
                             100) /
                             100
                      << "ms" << std::endl;
        }

        std::this_thread::sleep_for(ping_sleep_rate);
    }
}


int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <node>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const std::string host_name = {argv[1]};
    auto addrs = socket_wrapper::get_client_info(host_name, 0, SOCK_DGRAM);

    std::string addr_p;
    addr_p.resize(INET6_ADDRSTRLEN);
    auto si = reinterpret_cast<const sockaddr_in *>(addrs->ai_addr);
    inet_ntop(addrs->ai_family, &si->sin_addr, addr_p.data(), addr_p.size());

    std::cout << "Pinging \"" << host_name << "\" [" << addr_p << "]" << std::endl;

    int sock_type = SOCK_RAW;
    socket_wrapper::Socket sock = {AF_INET, sock_type, IPPROTO_ICMP};

    if (!sock)
    {
        std::cerr << "Can't create raw socket: " << sock_wrap.get_last_error_string() << std::endl;

        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Raw socket was created..." << std::endl;
    }

    std::cout << "Starting to send packets..." << std::endl;
    // Send pings continuously.
    send_ping(sock, host_name, *reinterpret_cast<const sockaddr_in *>(addrs->ai_addr), SOCK_RAW == sock_type);

    return EXIT_SUCCESS;
}
