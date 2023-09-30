#define _GNU_SOURCE

extern "C"
{
#include <netdb.h>
}

#include <iostream>
#include <string>
#include <system_error>
#include <vector>


/* Add requests for specified hostnames */
int add_request(const std::string &host, std::vector<gaicb *> &reqs)
{
    int nreqs_base = reqs.size();

    reqs.emplace_back(new gaicb{.ar_name = host.c_str()});

    /* Queue nreqs_base..nreqs requests. */

    auto ret = getaddrinfo_a(GAI_NOWAIT, &reqs.data()[nreqs_base], reqs.size() - nreqs_base, nullptr);

    if (ret)
    {
        throw std::system_error(EDOM, std::generic_category(), gai_strerror(ret));
    }

    return reqs.size() - 1;
}


/* Wait until at least one of specified requests completes */
void wait_requests(const std::vector<gaicb *> &wait_reqs)
{
    /* nullptr elements are ignored by gai_suspend(). */

    auto ret = gai_suspend(wait_reqs.data(), wait_reqs.size(), nullptr);

    if (ret)
    {
        throw std::system_error(EDOM, std::generic_category(), gai_strerror(ret));
    }

    for (size_t i = 0; i < wait_reqs.size(); ++i)
    {
        if (!wait_reqs[i]) continue;

        ret = gai_error(wait_reqs[i]);

        if (EAI_INPROGRESS == ret)
        {
            --i;
            continue;
        }

        std::cout << "[" << i << "] " << wait_reqs[i]->ar_name << ": " << (!ret ? "Finished" : gai_strerror(ret))
                  << std::endl;
    }
}


/* Cancel specified requests */
void cancel_request(int id, std::vector<gaicb *> &reqs)
{
    if (id >= reqs.size())
    {
        throw std::logic_error("Bad request number");
    }

    auto ret = gai_cancel(reqs[id]);
    std::cout << "[" << id << "] " << reqs[id]->ar_name << ": " << gai_strerror(ret) << std::endl;
}


/* List all requests */
void list_requests(const std::vector<gaicb *> &reqs)
{
    for (size_t i = 0; i < reqs.size(); ++i)
    {
        std::cout << "[" << i << "] " << reqs[i]->ar_name << ": ";
        auto ret = gai_error(reqs[i]);

        if (!ret)
        {
            addrinfo *res = reqs[i]->ar_result;
            char host[NI_MAXHOST];

            ret = getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
            if (ret)
            {
                throw std::system_error(EDOM, std::generic_category(), gai_strerror(ret));
            }
            std::cout << host;
        }
        else
        {
            std::cout << gai_strerror(ret);
        }
        std::cout << std::endl;
    }
}


int main(int argc, char *argv[])
{
    std::vector<std::string> sites = {"www.kernel.org", "www.yandex.ru", "www.google.com"};

    std::vector<gaicb *> requests;

    for (size_t i = 0; i < sites.size(); ++i)
    {
        add_request(sites[i], requests);
    }
    wait_requests(requests);
    list_requests(requests);

    exit(EXIT_SUCCESS);
}
