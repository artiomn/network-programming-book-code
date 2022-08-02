#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>
#include <linux/inet_diag.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <iostream>


struct netlink_request
{
    struct nlmsghdr nlh;
    // struct unix_diag_req udr;
    struct inet_diag_req_v2 idr;
};


static int send_query(int fd)
{
    sockaddr_nl nladdr =
    {
        .nl_family = AF_NETLINK
    };

    netlink_request req =
    {
        .nlh =
        {
            .nlmsg_len = sizeof(req),
            .nlmsg_type = SOCK_DIAG_BY_FAMILY,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP
        },
        .idr =
        {
            .sdiag_family = AF_INET,
            .sdiag_protocol = IPPROTO_TCP,
            .idiag_ext = 0,
            .pad = 0,
            .idiag_states = static_cast<__u32>(-1), //  1<<TCP_ESTABLISHED|1<<TCP_SYN_SENT|1<<TCP_FIN_WAIT1|1<<TCP_FIN_WAIT2|1<<TCP_CLOSE_WAIT|1<<TCP_LAST_ACK|1<<TCP_CLOSING|0x1, // ~(-1),
            .id = {0}
        }
    };

    struct iovec iov =
    {
        .iov_base = &req,
        .iov_len = sizeof(req)
    };

    struct msghdr msg =
    {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_controllen = 0,
        .msg_flags = 0
    };

    while (true)
    {
        if (sendmsg(fd, &msg, 0) < 0)
        {
            if (EINTR == errno) continue;

            perror("sendmsg");
            return -1;
        }

        return 0;
    }
}


static int print_diag(const struct inet_diag_msg *diag, unsigned int len)
{
    if (len < NLMSG_LENGTH(sizeof(*diag)))
    {
        fputs("short response\n", stderr);
        return -1;
    }

    if (diag->idiag_family != AF_INET && diag->idiag_family != AF_INET6)
    {
        fprintf(stderr, "unexpected family %u\n", diag->idiag_family);
        return -1;
    }

    unsigned int rta_len = len - NLMSG_LENGTH(sizeof(*diag));
    unsigned int peer = 0;
    size_t path_len = 0;
    struct tcp_info ti;

    for (struct rtattr *attr = (struct rtattr *) (diag + 1);
         RTA_OK(attr, rta_len); attr = RTA_NEXT(attr, rta_len))
    {
            std::cout << attr->rta_type << "\n";
        switch (attr->rta_type)
        {
            case INET_DIAG_TOS:
                if (!path_len)
                {
                    path_len = RTA_PAYLOAD(attr);
//                if (path_len > sizeof(path) - 1) path_len = sizeof(path) - 1;
//                memcpy(path, RTA_DATA(attr), path_len);
//                path[path_len] = '\0';
                }
            break;

            case INET_DIAG_INFO:
                if (RTA_PAYLOAD(attr) >= sizeof(peer))
                    peer = *static_cast<unsigned int *>(RTA_DATA(attr));
            break;
        }
    }

    std::string src_addr;
    src_addr.resize(INET_ADDRSTRLEN);

    std::string dst_addr;
    dst_addr.resize(INET_ADDRSTRLEN);

//    inet_ntop(AF_INET, idiag_src, &src_addr[0], src_addr.size());
//    printf("inode=%u", diag->udiag_ino);

    if (peer)
        printf(", peer=%u", peer);

//    if (path_len)
//        printf(", name=%s%s", *path ? "" : "@",
//                *path ? path : path + 1);

    std::cout << "\n";
    return 0;
}


static int receive_responses(int fd)
{
    long buf[8192 / sizeof(long)];
    struct sockaddr_nl nladdr;
    struct iovec iov =
    {
        .iov_base = buf,
        .iov_len = sizeof(buf)
    };

    int flags = 0;

    while (true)
    {
        struct msghdr msg =
        {
            .msg_name = &nladdr,
            .msg_namelen = sizeof(nladdr),
            .msg_iov = &iov,
            .msg_iovlen = 1
        };

        ssize_t ret = recvmsg(fd, &msg, flags);

        if (ret < 0)
        {
            if (EINTR == errno) continue;

            perror("recvmsg");
            return -1;
        }

        if (ret == 0) return 0;

        if (nladdr.nl_family != AF_NETLINK)
        {
            fputs("!AF_NETLINK\n", stderr);
            return -1;
        }

        const struct nlmsghdr *h = (struct nlmsghdr *) buf;

        if (!NLMSG_OK(h, ret))
        {
            fputs("!NLMSG_OK\n", stderr);
            return -1;
        }

        for (; NLMSG_OK(h, ret); h = NLMSG_NEXT(h, ret))
        {
            if (h->nlmsg_type == NLMSG_DONE) return 0;

            if (h->nlmsg_type == NLMSG_ERROR)
            {
                const struct nlmsgerr *err = static_cast<const nlmsgerr*>(NLMSG_DATA(h));

                if (h->nlmsg_len < NLMSG_LENGTH(sizeof(*err)))
                {
                    fputs("NLMSG_ERROR\n", stderr);
                }
                else
                {
                    errno = -err->error;
                    perror("NLMSG_ERROR");
                }

                return -1;
            }

            if (h->nlmsg_type != SOCK_DIAG_BY_FAMILY)
            {
                fprintf(stderr, "unexpected nlmsg_type %u\n",
                        (unsigned) h->nlmsg_type);
                return -1;
            }

            if (print_diag(static_cast<const inet_diag_msg*>(NLMSG_DATA(h)), h->nlmsg_len))
                return -1;
        }
    }
}


int main(void)
{
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);

    if (fd < 0)
    {
        perror("socket");
        return 1;
    }

    int result = send_query(fd);


//    if (!result) 
receive_responses(fd);

std::cout << result << std::endl;

    close(fd);
    return result;
}

