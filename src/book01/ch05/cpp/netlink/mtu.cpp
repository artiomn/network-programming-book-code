extern "C"
{
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/socket.h>
}

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <iostream>


const auto BUF_SIZE = 1024;


int main(int argc, const char *const argv[])
{
    if (argc < 3)
    {
        std::cout << argv[0] << " <iface_name> <mtu>" << std::endl;
        return EXIT_FAILURE;
    }

    const auto iface_index = if_nametoindex(argv[1]);
    const unsigned int mtu = std::stoi(argv[2]);

    struct
    {
        nlmsghdr nh;
        ifinfomsg if_msg;
        char attrbuf[BUF_SIZE];
    } req = {0};

    int rtnetlink_sk = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    if (rtnetlink_sk < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nh.nlmsg_type = RTM_NEWLINK;

    req.if_msg.ifi_family = AF_UNSPEC;
    req.if_msg.ifi_index = iface_index;

    req.if_msg.ifi_change = 0;

    rtattr *rta = reinterpret_cast<rtattr *>(reinterpret_cast<char *>(&req) + NLMSG_ALIGN(req.nh.nlmsg_len));

    rta->rta_type = IFLA_MTU;
    rta->rta_len = RTA_LENGTH(sizeof(unsigned int));

    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_LENGTH(rta->rta_len);
    std::copy_n(reinterpret_cast<const char *>(&mtu), sizeof(mtu), static_cast<char *>(RTA_DATA(rta)));

    if (send(rtnetlink_sk, &req, req.nh.nlmsg_len, 0) < 0)
    {
        perror("send");
        return EXIT_FAILURE;
    }
}
