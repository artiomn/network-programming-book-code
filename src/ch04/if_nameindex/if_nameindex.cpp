#include <cerrno>
#include <iostream>
#include <memory>
#include <net/if.h>


int main(int argc, const char * const argv[])
{
    std::unique_ptr<struct if_nameindex, decltype(&if_freenameindex)> if_ni(if_nameindex(), &if_freenameindex);

    if (nullptr == if_ni)
    {
        std::perror("if_nameindex");
        return EXIT_FAILURE;
    }

    for (auto i = if_ni.get(); !(0 == i->if_index && nullptr == i->if_name); ++i)
        std::cout
            << i->if_index << ": "
            << i->if_name << "\n";
    std::cout << std::endl;

   return EXIT_SUCCESS;
}

