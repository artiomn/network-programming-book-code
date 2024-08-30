extern "C"
{
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
}

#include <array>
#include <iostream>


int main()
{
    std::array<unsigned char, 256> buf;
    unsigned char *dnptrs = nullptr;
    unsigned char *lastdnptr = nullptr;

    if (auto result = dn_comp("www.google.com", buf.data(), buf.size(), &dnptrs, &lastdnptr); result != -1)
    {
        std::cout << "Len = " << result << "\n"
                  << std::string(buf.begin(), buf.begin() + result) << "\n"
                  << "DP = " << (dnptrs ? reinterpret_cast<const char *>(dnptrs) : "nullptr") << "\n"
                  << "LDP = " << (lastdnptr ? reinterpret_cast<const char *>(lastdnptr) : "nullptr") << std::endl;

        std::array<char, 256> exp_buf;

        if ((result = dn_expand(buf.data(), buf.data() + result, buf.data(), exp_buf.data(), exp_buf.size())) != -1)
        {
            std::cout << "Len = " << result << "\n"
                      << std::string(exp_buf.begin(), exp_buf.begin() + result) << "\n"
                      << std::endl;
            return EXIT_SUCCESS;
        }

        perror("dn_expand");
        return EXIT_FAILURE;
    }
    perror("dn_comp");

    return EXIT_FAILURE;
}
