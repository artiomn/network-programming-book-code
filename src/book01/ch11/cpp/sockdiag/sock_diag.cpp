extern "C"
{
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/inet_diag.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>
#include <linux/tcp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
    // Don't use this here!
    // #include <netinet/tcp.h>
}

#include <cerrno>
#include <iostream>
#include <memory>


struct netlink_request
{
    nlmsghdr nlh;
    inet_diag_req_v2 irh;
};


void send_query(int fd)
{
    sockaddr_nl nladdr = {.nl_family = AF_NETLINK};

    netlink_request req = {
        .nlh = {.nlmsg_len = sizeof(req), .nlmsg_type = SOCK_DIAG_BY_FAMILY, .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP},
        .irh = {
            .sdiag_family = AF_INET,
            .sdiag_protocol = IPPROTO_TCP,
            .idiag_ext = 1 << (INET_DIAG_TOS - 1) | 1 << (INET_DIAG_INFO - 1),
            .pad = 0,
            // 1 << TCP_ESTABLISHED | 1 << TCP_SYN_SENT | 1 << TCP_FIN_WAIT1| \
            // 1 << TCP_FIN_WAIT2 | 1 << TCP_CLOSE_WAIT | 1 << TCP_LAST_ACK | 1 << TCP_CLOSING
            .idiag_states = static_cast<__u32>(-1),
            .id = {0}}};

    iovec iov = {.iov_base = &req, .iov_len = sizeof(req)};

    msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_controllen = 0,
        .msg_flags = 0};

    while (true)
    {
        if (sendmsg(fd, &msg, 0) < 0)
        {
            if (EINTR == errno) continue;

            throw std::system_error(errno, std::generic_category(), "sendmsg");
        }
        break;
    }
}


void print_diag(const inet_diag_msg *diag, unsigned int len)
{
    if (len < NLMSG_LENGTH(sizeof(*diag)))
    {
        throw std::system_error(errno, std::generic_category(), "short response");
    }

    if (diag->idiag_family != AF_INET && diag->idiag_family != AF_INET6)
    {
        throw std::logic_error("unexpected protocol family");
    }

    std::string src_addr(INET_ADDRSTRLEN, 0);
    std::string dst_addr(INET_ADDRSTRLEN, 0);

    inet_ntop(diag->idiag_family, diag->id.idiag_src, &src_addr[0], src_addr.size());
    inet_ntop(diag->idiag_family, diag->id.idiag_dst, &dst_addr[0], src_addr.size());

    std::cout << src_addr << ":" << ntohs(diag->id.idiag_sport) << " -> " << dst_addr << ":"
              << ntohs(diag->id.idiag_dport);

    if (diag->id.idiag_if)
    {
        // Socket was bound to the interface.
        const std::unique_ptr<struct if_nameindex, decltype(&if_freenameindex)> if_ni(
            if_nameindex(), &if_freenameindex);

        if (nullptr == if_ni)
        {
            throw std::system_error(errno, std::generic_category(), "if_nameindex");
        }

        std::string if_name;

        for (auto i = if_ni.get(); !(0 == i->if_index && nullptr == i->if_name); ++i)
        {
            if (i->if_index == diag->id.idiag_if)
            {
                if_name = i->if_name;
                break;
            }
        }

        std::cout << " [" << if_name << "]";
    }

    std::cout << ":\n";

    unsigned int rta_len = len - NLMSG_LENGTH(sizeof(*diag));

    // Attributes.
    for (const rtattr *attr = reinterpret_cast<const rtattr *>(diag + 1); RTA_OK(attr, rta_len);
         attr = RTA_NEXT(attr, rta_len))
    {
        switch (attr->rta_type)
        {
            case INET_DIAG_TOS:
                if (RTA_PAYLOAD(attr) != 1) throw std::logic_error("TOS length error");
                std::cout << "  TOS: " << +*static_cast<const uint8_t *>(RTA_DATA(attr)) << "\n";
                break;
            case INET_DIAG_INFO:
            {
                auto data_size = RTA_PAYLOAD(attr);
                if (data_size != sizeof(tcp_info))
                    throw std::logic_error("tcp_info length error, check if you use correct header (linux/tcp.h)");

                tcp_info ti;
                auto data = static_cast<const uint8_t *>(RTA_DATA(attr));

                std::copy(data, data + data_size, reinterpret_cast<uint8_t *>(&ti));

                std::cout << "  Lost packets: " << ti.tcpi_lost << "\n"
                          << "  Retransmits: " << +ti.tcpi_retransmits << "\n"
                          << "  RTT: " << ti.tcpi_rtt << "\n";
                break;
            }
            case INET_DIAG_SHUTDOWN:
                if (RTA_PAYLOAD(attr) != 1) throw std::logic_error("Shutdown flags length error");
                std::cout << "  Shutdown flags: " << +*static_cast<uint8_t *>(RTA_DATA(attr)) << "\n";
                break;
            case INET_DIAG_CGROUP_ID:
                if (RTA_PAYLOAD(attr) != sizeof(uint64_t)) throw std::logic_error("CGroup length error");
                std::cout << "  Control group: " << *static_cast<const uint64_t *>(RTA_DATA(attr)) << "\n";
                break;
            case INET_DIAG_SOCKOPT:
                if (RTA_PAYLOAD(attr) != sizeof(inet_diag_sockopt))
                    throw std::logic_error("Sockopt flags length error");
                std::cout << "  SOCKOPT\n";
                break;
            default:
                std::cerr << "  Unknown attribute: "
                          << "0x" << std::hex << attr->rta_type << "\n";
        }
    }

    std::cout << "\n";
}


void print_responses(int fd)
{
    std::byte buf[4096];
    sockaddr_nl nladdr;
    iovec iov = {.iov_base = buf, .iov_len = sizeof(buf)};

    int flags = 0;

    while (true)
    {
        msghdr msg = {.msg_name = &nladdr, .msg_namelen = sizeof(nladdr), .msg_iov = &iov, .msg_iovlen = 1};

        ssize_t ret = recvmsg(fd, &msg, flags);

        if (ret < 0)
        {
            if (EINTR == errno) continue;

            throw std::system_error(errno, std::generic_category(), "recvmsg");
        }

        if (ret == 0) return;

        if (nladdr.nl_family != AF_NETLINK)
        {
            throw std::logic_error("nl_family != AF_NETLINK");
        }

        const nlmsghdr *h = reinterpret_cast<const nlmsghdr *>(buf);

        if (!NLMSG_OK(h, ret))
        {
            throw std::system_error(errno, std::generic_category(), "not NLMSG_OK");
        }

        for (; NLMSG_OK(h, ret); h = NLMSG_NEXT(h, ret))
        {
            if (h->nlmsg_type == NLMSG_DONE) return;

            if (h->nlmsg_type == NLMSG_ERROR)
            {
                const nlmsgerr *err = static_cast<const nlmsgerr *>(NLMSG_DATA(h));

                if (h->nlmsg_len < NLMSG_LENGTH(sizeof(*err)))
                {
                    throw std::system_error(errno, std::generic_category(), "NLMSG_ERROR");
                }
                else
                {
                    errno = -err->error;
                    throw std::system_error(errno, std::generic_category(), "NLMSG_ERROR");
                }
            }

            if (h->nlmsg_type != SOCK_DIAG_BY_FAMILY)
            {
                throw std::system_error(errno, std::generic_category(), "unexpected nlmsg_type");
            }

            print_diag(static_cast<const inet_diag_msg *>(NLMSG_DATA(h)), h->nlmsg_len);
        }
    }
}


int main(void)
{
    try
    {
        int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);

        if (fd < 0)
        {
            throw std::system_error(errno, std::generic_category(), "socket");
        }

        try
        {
            send_query(fd);
            print_responses(fd);
            close(fd);
        }
        catch (...)
        {
            close(fd);
            throw;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
