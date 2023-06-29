extern "C"
{
#include <asm/types.h>
#include <libnetlink.h>
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
        // cppcheck-suppress unusedStructMember
        char attrbuf[BUF_SIZE];
    } req = {0};

    rtnl_handle rth = {0};

    if (rtnl_open(&rth, 0) < 0)
    {
        perror("rtnl_open");
        rtnl_close(&rth);
        return EXIT_FAILURE;
    }

    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nh.nlmsg_type = RTM_NEWLINK;

    req.if_msg.ifi_family = AF_UNSPEC;
    req.if_msg.ifi_index = iface_index;

    req.if_msg.ifi_change = 0;

    addattr32(&req.nh, BUF_SIZE, IFLA_MTU, mtu);
    //    std::copy_n(reinterpret_cast<const char *>(&mtu), sizeof(mtu), static_cast<char *>(RTA_DATA(rta)));

    if (rtnl_send(&rth, &req, req.nh.nlmsg_len))
    {
        perror("send");
        return EXIT_FAILURE;
    }
}
