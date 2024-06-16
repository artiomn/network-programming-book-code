#include <iostream>

extern "C"
{
#if !defined(_XOPEN_SOURCE)
#    define _XOPEN_SOURCE
#endif
#include <limits.h>
#include <unistd.h>
}


int main()
{
    auto buf_count = sysconf(_SC_IOV_MAX);

    if (-1 == buf_count)
    {
        perror("sysconf");
#if defined(IOV_MAX)
        buf_count = IOV_MAX;
        std::cout << "Buffers count got via IOV_MAX\n";
#else
        std::cerr << "Can't get IOV_MAX" << std::endl;
        return EXIT_FAILURE;
#endif
    }
    else
    {
        std::cout << "Buffers count got via sysconf()\n";
    }

#if defined(IOV_MAX)
    std::cout << "IOV_MAX = " << IOV_MAX << "\n";
#endif
    std::cout << "Buffers count = " << buf_count << std::endl;

    return EXIT_SUCCESS;
}
