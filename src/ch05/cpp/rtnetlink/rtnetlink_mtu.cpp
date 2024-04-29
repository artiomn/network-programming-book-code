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

    try
    {
        if (rtnl_open(&rth, 0) < 0)
        {
            throw std::system_error(errno, std::system_category(), "rtnl_open()");
        }

        addattr32(&req.nh, BUF_SIZE, IFLA_MTU, mtu);
        //    std::copy_n(reinterpret_cast<const char *>(&mtu), sizeof(mtu), static_cast<char *>(RTA_DATA(rta)));

        if (rtnl_send(&rth, &req, req.nh.nlmsg_len))
        {
            rtnl_close(&rth);
            throw std::system_error(errno, std::system_category(), "rtnl_send()");
        }
        rtnl_close(&rth);
        std::cout << "MTU set successful." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
