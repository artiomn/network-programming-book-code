extern "C"
{
#include <asm/types.h>
#include <iproute2/libnetlink.h>
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
    } req = {
        .nh =
            {
                .nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg)),
                .nlmsg_type = RTM_NEWLINK,
                .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
            },
        .if_msg = {.ifi_family = AF_UNSPEC, .ifi_index = iface_index, .ifi_change = 0}};

    rtnl_handle rth = {0};

    if (rtnl_open(&rth, 0) < 0)
    {
        perror("rtnl_open");
        rtnl_close(&rth);
        return EXIT_FAILURE;
    }

    addattr32(&req.nh, BUF_SIZE, IFLA_MTU, mtu);
    //    std::copy_n(reinterpret_cast<const char *>(&mtu), sizeof(mtu), static_cast<char *>(RTA_DATA(rta)));

    if (rtnl_send(&rth, &req, req.nh.nlmsg_len))
    {
        perror("send");
        return EXIT_FAILURE;
    }
}
