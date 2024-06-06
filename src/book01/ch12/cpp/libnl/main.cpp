extern "C"
{
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/route/link.h>
}

#include <cassert>
#include <iostream>
#include <memory>


int main(int argc, const char *const argv[])
{
    if (argc != 3)
    {
        std::cerr << argv[0] << " <if_name> <a|d>" << std::endl;
        return EXIT_FAILURE;
    }

    assert(argv[1]);
    const std::string if_name{argv[1]};

    assert(argv[2]);
    const std::string if_action{argv[2]};

    const std::unique_ptr<nl_sock, decltype(&nl_socket_free)> sock{nl_socket_alloc(), &nl_socket_free};
    if (!sock)
    {
        std::cerr << "Can't allocate socket!" << std::endl;
        return EXIT_FAILURE;
    }

    nl_cache *cache = nullptr;
    int error_code = 1;

    try
    {
        if ((error_code = nl_connect(sock.get(), NETLINK_ROUTE)) < 0)
        {
            throw std::system_error(error_code, std::system_category(), "Unable to connect socket");
        }

        std::cout << "Socket connected." << std::endl;

        if ((error_code = rtnl_link_alloc_cache(sock.get(), AF_UNSPEC, &cache)) < 0)
        {
            throw std::system_error(error_code, std::system_category(), "Unable to allocate cache");
        }

        const std::unique_ptr<rtnl_link, decltype(&rtnl_link_put)> link{
            rtnl_link_get_by_name(cache, if_name.c_str()), &rtnl_link_put};
        if (!link)
        {
            throw std::system_error(error_code, std::iostream_category(), "Interface was not found");
        }
        std::cout << "\"" << if_name << "\" interface acquired" << std::endl;

        std::cout << "Current \"" << if_name
                  << "\" status: " << ((rtnl_link_get_flags(link.get()) & IFF_UP) ? "up" : "down") << std::endl;

        const std::unique_ptr<rtnl_link, decltype(&rtnl_link_put)> change{rtnl_link_alloc(), &rtnl_link_put};
        if ("a" == if_action)
            rtnl_link_set_flags(change.get(), IFF_UP);
        else if ("d" == if_action)
            rtnl_link_unset_flags(change.get(), IFF_UP);
        else
            std::cerr << "Unknown action type!" << std::endl;

        if ((error_code = rtnl_link_change(sock.get(), link.get(), change.get(), 0)) < 0)
        {
            throw std::system_error(error_code, std::iostream_category(), "Unable to change interface status");
        }
    }
    catch (const std::system_error &e)
    {
        nl_perror(e.code().value(), e.what());
    }

    if (cache) nl_cache_free(cache);

    return error_code;
}
