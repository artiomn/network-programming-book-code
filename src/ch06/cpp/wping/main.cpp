#include <icmpapi.h>
#include <ipexport.h>
#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>

#include <cerrno>
#include <iostream>
#include <memory>
#include <vector>


// Ping function for return ping value.
enum class t_ping_func
{
    tpf_avg,
    tpf_max,
    tpf_min
};


// Ping options.

enum ping_options : unsigned char
{
    // Checking availability.
    PING_OPT_AVAIL = 0x01,
    // Up verbosity level.
    PING_OPT_VERBOSE = 0x02,
    // Don't call Sleep() between pings.
    PING_OPT_NOSLEEP = 0x04
};

//
// Default values.
//

// Delay between pings in ms.
const auto PING_DEF_DELAY = 1000;
// Packets count.
const auto PING_DEF_COUNT = 3;
// Packet size in bytes.
const auto PING_DEF_DATA_SIZE = 32;
// Default ping function.
const auto PING_DEF_FUNC = t_ping_func::tpf_max;

//
// Limitations.
//

const auto PING_MAX_DATA_SIZE = 0xffe0;
// Maximum ping time (maximum retransmission time about 4 min.)
const auto PING_MAX_TIME = 250000;


struct t_ping_data
{
    char *host;
    DWORD opts;
    t_ping_func ping_func;
    int count;
    int delay;
    size_t buf_sz;
    char pattern;
};


PICMP_ECHO_REPLY create_reply_buffer(
    char *buf, unsigned short data_sz, const std::vector<char> &reply_holder)  // NOLINT
{
    auto p_reply = new (reply_holder.data()) ICMP_ECHO_REPLY;

    // Already zeroed.
    // std::fill(reinterpret_cast<char*>(p_reply), reinterpret_cast<char*>(p_reply) + sizeof(ICMP_ECHO_REPLY), 0);

    p_reply->Data = buf;
    p_reply->DataSize = data_sz;

    return p_reply;
}


int get_end_time(const DWORD end_time, const t_ping_data *const wping, const DWORD rtt)
{
    switch (wping->ping_func)
    {
        case t_ping_func::tpf_max:
            return max(rtt, end_time);
            break;
        case t_ping_func::tpf_avg:
            return end_time + rtt;
            break;
        case t_ping_func::tpf_min:
            return min(rtt, end_time);
            break;
        default:
            break;
    }
    return 0;
}


bool ping(const t_ping_data *const wping)
{
    // IP options.
    IP_OPTION_INFORMATION ip_info;
    std::vector<char> buf;

    bool ret_status = false;
    // Reply time.
    int end_time;
    // Ping with responses.
    DWORD good_packets_cnt = 0;

    if (wping->buf_sz > 0)
    {
        if (wping->buf_sz > PING_MAX_DATA_SIZE)
        {
            std::cerr << "Error: maximum ping data size = " << PING_MAX_DATA_SIZE
                      << ", but you asked size = " << wping->buf_sz << std::endl;
            throw std::logic_error("Maximum ping data size exceeded!");
        }

        buf.resize(wping->buf_sz);

        if (wping->opts & PING_OPT_VERBOSE) std::cout << "Data pattern: 0x" << wping->pattern << std::endl;

        std::fill(buf.begin(), buf.begin() + wping->buf_sz, wping->pattern);
    }

    std::vector<char> reply_buf(sizeof(ICMP_ECHO_REPLY) + buf.size());

    auto echo_reply = create_reply_buffer(buf.data(), static_cast<unsigned short>(buf.size()), reply_buf);  // NOLINT
    // Get an ICMP echo request handle.
    HANDLE hndlFile = IcmpCreateFile();

    if (INVALID_HANDLE_VALUE == hndlFile)
    {
        throw std::system_error(errno, std::system_category(), "IcmpCreateFile() error!");
    }

    auto addrs = socket_wrapper::get_client_info(wping->host, 0, SOCK_STREAM, AF_INET);
    addrinfo *ai = addrs.get();

    while (ai->ai_family != AF_INET)
    {
        if (!ai->ai_next) throw std::logic_error("Cant resolve IPv4 for the host!");
        ai = ai->ai_next;
    }

    // Internet address structure.
    sockaddr_in ia_dest;

    std::copy(ai->ai_addr, ai->ai_addr + 1, reinterpret_cast<sockaddr *>(&ia_dest));
    if (INADDR_NONE == ia_dest.sin_addr.s_addr || ia_dest.sin_family != AF_INET)
    {
        if (wping->opts & PING_OPT_VERBOSE) std::cerr << "Can't lookup destination!" << std::endl;
        throw std::logic_error("Can't lookup destination!");
    }

    char ip_buf[16];

    while (true)
    {
        // Lookup destination
        // Use inet_addr() to determine if we're dealing with a name
        // or an address.

        // Set some reasonable default values.
        ip_info.Ttl = 255;
        ip_info.Tos = 0;
        ip_info.Flags = 0;
        ip_info.OptionsSize = 0;
        ip_info.OptionsData = nullptr;

        if (wping->opts & PING_OPT_VERBOSE)
        {
            std::cout << "Count = " << wping->count << "\nSize  = " << wping->buf_sz + sizeof(ICMP_ECHO_REPLY)
                      << "\nDelay = " << wping->delay << "\nSending to \""
                      << inet_ntop(AF_INET, &ia_dest.sin_addr, ip_buf, sizeof(ip_buf)) << "\"\n"
                      << std::endl;
        }

        end_time = (t_ping_func::tpf_min == wping->ping_func) ? PING_MAX_TIME : 0;
        for (int i = 0; i < wping->count; ++i)
        {
            echo_reply->Status = IP_SUCCESS;
            // Reqest an ICMP echo:
            // icmpSendEcho(Handle from IcmpCreateFile(), Destination IP address,
            //              Pointer to buffer to send, Size of buffer in bytes,
            //              Request options, Reply buffer, Time to wait in milliseconds).
            // Replies count.
            DWORD rep_cnt = IcmpSendEcho(
                hndlFile, reinterpret_cast<const sockaddr_in *>(&ia_dest)->sin_addr.s_addr, buf.data(),
                static_cast<WORD>(buf.size()), &ip_info, echo_reply,
                static_cast<DWORD>(sizeof(ICMP_ECHO_REPLY) + buf.size()), wping->delay);
            if (!(wping->opts & PING_OPT_NOSLEEP)) Sleep(wping->delay);

            if (rep_cnt > 0 && (IP_SUCCESS == echo_reply->Status))
            {
                while (rep_cnt--)
                {
                    end_time = get_end_time(end_time, wping, echo_reply->RoundTripTime);
                    if (wping->opts & PING_OPT_VERBOSE)
                    {
                        std::cout << "Received from: "
                                  << inet_ntop(AF_INET, &echo_reply->Address, ip_buf, sizeof(ip_buf))
                                  << "\nStatus: " << echo_reply->Status
                                  << "\nRoundtrip time: " << echo_reply->RoundTripTime << "ms"
                                  << "\nTTL: " << static_cast<int>(echo_reply->Options.Ttl)
                                  << "\nBytes: " << echo_reply->DataSize << "\n"
                                  << std::endl;
                    }
                    ++good_packets_cnt;
                }
            }
            else
            {
                if (wping->opts & PING_OPT_VERBOSE)
                {
                    std::cout << "Host " << wping->host << " (resolved:\""
                              << inet_ntop(AF_INET, &ia_dest.sin_addr, ip_buf, sizeof(ip_buf))
                              << "\") doesn't response!" << std::endl;
                }
                end_time += get_end_time(end_time, wping, wping->delay);
            }
        }

        ret_status = good_packets_cnt != 0;
        break;
    }

    if (!(wping->opts & PING_OPT_AVAIL))
    {
        if (ret_status)
        {
            std::cout << ((t_ping_func::tpf_avg != wping->ping_func)
                              ? end_time
                              : static_cast<float>(end_time) / (good_packets_cnt + 1))
                      << std::endl;
        }
        else
            std::cout << "-1" << std::endl;
    }

    IcmpCloseHandle(hndlFile);
    return ret_status;
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <hostname>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sw;
    struct t_ping_data wping;

    wping.opts = PING_OPT_VERBOSE;  // PING_OPT_NOSLEEP, PING_OPT_AVAIL
    wping.count = PING_DEF_COUNT;
    wping.buf_sz = PING_DEF_DATA_SIZE;
    wping.delay = PING_DEF_DELAY;
    wping.ping_func = PING_DEF_FUNC;

    wping.host = argv[1];
    wping.pattern = 'a';

    if (wping.opts & PING_OPT_AVAIL)
        std::cout << (ping(&wping) ? 1 : 0) << std::endl;
    else
        ping(&wping);

    return EXIT_SUCCESS;
}
