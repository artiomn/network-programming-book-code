extern "C"
{
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/route/link.h>
}

#include <iostream>


int main(int argc, const char *const *argv)
{
    if (argc != 3)
    {
        std::cerr << argv[0] << " <if_name> <a|d>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string if_name{argv[1]};
    const std::string if_action{argv[2]};

    nl_sock *sock = nl_socket_alloc();
    nl_cache *cache;
    int error_code = 1;

    if (!sock)
    {
        std::cerr << "Can't allocate socket!" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        if ((error_code = nl_connect(sock, NETLINK_ROUTE)) < 0)
        {
            throw std::system_error(error_code, std::system_category(), "Unable to connect socket");
        }

        std::cout << "Socket connected." << std::endl;
        rtnl_link *change;

        if ((error_code = rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache)) < 0)
        {
            throw std::system_error(error_code, std::system_category(), "Unable to allocate cache");
        }

        rtnl_link *link = rtnl_link_get_by_name(cache, if_name.c_str());
        if (!link)
        {
            throw std::system_error(error_code, std::iostream_category(), "Interface was not found");
        }
        std::cout << "\"" << if_name << "\" interface acquired" << std::endl;

        std::cout << "Current \"" << if_name << "\" status: " << ((rtnl_link_get_flags(link) & IFF_UP) ? "up" : "down")
                  << std::endl;

        change = rtnl_link_alloc();
        if ("a" == if_action)
            rtnl_link_set_flags(change, IFF_UP);
        else if ("d" == if_action)
            rtnl_link_unset_flags(change, IFF_UP);
        else
            std::cerr << "Unknown action type!" << std::endl;

        if ((error_code = rtnl_link_change(sock, link, change, 0)) < 0)
        {
            throw std::system_error(error_code, std::iostream_category(), "Unable to change interface status");
        }
    }
    catch (const std::system_error &e)
    {
        nl_perror(e.code().value(), e.what());
    }

    nl_socket_free(sock);

    return error_code;
}
