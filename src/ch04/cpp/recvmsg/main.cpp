#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


class AncilDataProcessor
{
public:
    void recv_cb(evutil_socket_t fd, short event, void *arg)
    {
        struct msghdr msg;
        struct iovec iov[1];
        ssize_t len;
        int flags = 0;
        struct cmsghdr *cmptr;
        sockaddr_union_t sau;

        struct unp_in_pktinfo
        {
            union addr_union
            {
                struct in_addr  ipi_addr;
                struct in6_addr ipi_addr6;
            } au;
            int ipi_ifindex;
        } pkt;

        union
        {
            struct cmsghdr cm;
            u_int8_t pktinfo_sizer[sizeof(struct cmsghdr) + 1024];
        } control_un;

        uint16_t port = 0;
        char dst[INET6_ADDRSTRLEN], src[INET6_ADDRSTRLEN];
        u_int8_t *buf = calloc(1, 4096);

        msg.msg_control = &control_un;
        msg.msg_controllen = sizeof(control_un);
        msg.msg_flags = 0;

        msg.msg_name = &sau.sa;
        msg.msg_namelen = sizeof(sau);
        iov[0].iov_base = buf;
        iov[0].iov_len = MAX_PKT_SIZE - 1;
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        len = recvmsg(fd, &msg, flags);
        if (len < 0)
        {
            perror("recvmsg");
            goto done;
        }
        else if (len == 0)
        {
            fprintf(stderr, "recvmsg len 0, Connection closed");
            goto done;
        }

        bzero(&pkt, sizeof(struct unp_in_pktinfo));
        flags = msg.msg_flags;

        check_msg();

        for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL;
             cmptr = CMSG_NXTHDR(&msg, cmptr))
        {
            ip_pktinfo_process();
            ip6_pktinfo_process();
            recv_stdaddr_process();
            recvif_process();
        }
    }

protected:
    void check_msg()
    {
        if (msg.msg_controllen < sizeof(struct cmsghdr))
        {
            fprintf(stderr, "received truncated msg len: %d", msg.msg_controllen);
        }

        if (flags & MSG_TRUNC)
        {
            fprintf(stderr, "received truncated datagram: 0x%0x", flags);
        }

        if (flags & MSG_CTRUNC)
        {
            fprintf(stderr, "received truncated control info: 0x%0x", flags);
        }
    }

    bool ip_pktinfo_process()
    {
        if ((SOL_IP == cmptr->cmsg_level) && (IP_PKTINFO == cmptr->cmsg_type))
        {
            struct in_pktinfo *i = (struct in_pktinfo *) CMSG_DATA(cmptr);
            memcpy(&pkt.au.ipi_addr, &i->ipi_addr, sizeof(struct in_addr));
            pkt.ipi_ifindex = i->ipi_ifindex;
            return true;
        }
        return false;
    }

    bool ip6_pktinfo_process()
    {
        if (cmptr->cmsg_level == IPPROTO_IPV6 &&
            cmptr->cmsg_type == IPV6_PKTINFO)
        {
            struct in6_pktinfo *pkt_info;

            pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmptr);
            memcpy(&pkt.au.ipi_addr6, &pkt_info->ipi6_addr, sizeof(struct in6_addr));
            pkt.ipi_ifindex = pkt_info->ipi6_ifindex;
            return true;
        }
        return false;
    }

    bool recv_stdaddr_process()
    {
        if (cmptr->cmsg_level == IPPROTO_IP &&
            cmptr->cmsg_type == IP_RECVDSTADDR)
        {

            memcpy(&pkt.au.ipi_addr, CMSG_DATA(cmptr), sizeof(struct in_addr));
            return true;
            continue;
        }
        return false;
    }

    bool recvif_process()
    {
        if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
        {
            struct sockaddr_dl *sdl;

            sdl = (struct sockaddr_dl *)CMSG_DATA(cmptr);
            pkt.ipi_ifindex = sdl->sdl_index;
            return true;
        }
        return false;
    }

private:

};


int main(int argc, const char * const argv[])
{

   exit(EXIT_SUCCESS);
}
