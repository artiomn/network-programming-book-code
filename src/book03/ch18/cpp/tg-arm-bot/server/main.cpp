#include <csignal>
#include <iostream>
#include <string>

#include "server_tg.h"
#include "server_web.h"
#include "business_logic.h"


int main(int argc, const char *const argv[])
{
    signal(SIGINT, [](int s)
    {
        std::cout << "SIGINT got" << std::endl;
        exit(EXIT_SUCCESS);
    });

    const auto token = getenv("TOKEN");
    unsigned short port = 8080;

    if (!token)
    {
        std::cerr
            << "Token was not passed!\n"
            << "Set token anywhere in the environment.\n"
            << "I.e.: TOKEN=1234567890:AAFuiAr-mRDfKh5LV7NQbE-NMkUXPtVI5_0 "
            << argv[0]
            << std::endl;
        return EXIT_FAILURE;
    }

    if (2 == argc)
    {
        port = std::stoi(argv[1]);
    }
    else if (argc > 2)
    {
        std::cerr
            << argv[0] << " [web server port]"
            << std::endl;

        return EXIT_FAILURE;
    }

    ServerBusinessLogic bl(token, port);
    bl.run();

    return EXIT_SUCCESS;
}

