/* This example is placed in the public domain. */
extern "C"
{
#include <arpa/inet.h>
#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
}

#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>


static int data_attr_cb(const nlattr *attr, void *data)
{
    const nlattr **tb = static_cast<const nlattr **>(data);
    int type = mnl_attr_get_type(attr);

    /* skip unsupported attribute in user-space */
    if (mnl_attr_type_valid(attr, IFA_MAX) < 0) return MNL_CB_OK;

    switch (type)
    {
        case IFA_ADDRESS:
            if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
            {
                perror("mnl_attr_validate");
                return MNL_CB_ERROR;
            }
            break;
    }
    tb[type] = attr;
    return MNL_CB_OK;
}


static int data_cb(const nlmsghdr *nlh, void *data)
{
    nlattr *tb[IFA_MAX + 1] = {};
    ifaddrmsg *ifa = static_cast<ifaddrmsg *>(mnl_nlmsg_get_payload(static_cast<const nlmsghdr *>(nlh)));

    std::cout << "index = " << ifa->ifa_index << ", family = " << +ifa->ifa_family;

    mnl_attr_parse(nlh, sizeof(*ifa), data_attr_cb, tb);
    std::cout << " addr = ";
    if (tb[IFA_ADDRESS])
    {
        void *addr = mnl_attr_get_payload(tb[IFA_ADDRESS]);
        char out[INET6_ADDRSTRLEN];

        if (inet_ntop(ifa->ifa_family, addr, out, sizeof(out))) std::cout << out << " ";
    }
    std::cout << " scope = ";
    switch (ifa->ifa_scope)
    {
        case 0:
            std::cout << "global ";
            break;
        case 200:
            std::cout << "site ";
            break;
        case 253:
            std::cout << "link ";
            break;
        case 254:
            std::cout << "host ";
            break;
        case 255:
            std::cout << "nowhere ";
            break;
        default:
            std::cout << ifa->ifa_scope << " ";
            break;
    }

    std::cout << std::endl;
    return MNL_CB_OK;
}


int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <inet|inet6>" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> buf(MNL_SOCKET_BUFFER_SIZE);

    unsigned int seq;
    nlmsghdr *nlh = mnl_nlmsg_put_header(buf.data());

    nlh->nlmsg_type = RTM_GETADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = seq = time(nullptr);

    rtgenmsg *rt = static_cast<rtgenmsg *>(mnl_nlmsg_put_extra_header(static_cast<nlmsghdr *>(nlh), sizeof(rtgenmsg)));

    std::string s_type = argv[1];

    if ("inet" == s_type)
        rt->rtgen_family = AF_INET;
    else if ("inet6" == s_type)
        rt->rtgen_family = AF_INET6;

    mnl_socket *nl = mnl_socket_open(NETLINK_ROUTE);
    if (!nl)
    {
        perror("mnl_socket_open");
        return EXIT_FAILURE;
    }

    try
    {
        if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
        {
            throw std::system_error(errno, std::system_category(), "mnl_socket_bind");
        }

        unsigned int portid = mnl_socket_get_portid(nl);

        if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
        {
            throw std::system_error(errno, std::system_category(), "mnl_socket_sendto");
        }

        int ret = mnl_socket_recvfrom(nl, buf.data(), buf.size());
        while (ret > 0)
        {
            ret = mnl_cb_run(buf.data(), ret, seq, portid, data_cb, nullptr);
            if (ret <= MNL_CB_STOP) break;
            ret = mnl_socket_recvfrom(nl, buf.data(), buf.size());
        }
        if (-1 == ret)
        {
            throw std::system_error(ret, std::system_category(), "error");
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << "Error [" << e.code().value() << "]: " << e.what() << std::endl;
    }

    mnl_socket_close(nl);

    return EXIT_SUCCESS;
}
