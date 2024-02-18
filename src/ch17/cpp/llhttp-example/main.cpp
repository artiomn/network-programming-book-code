#include <llhttp.h>

#include <iomanip>
#include <iostream>
#include <string>


int handle_on_message_complete(llhttp_t* parser)
{
    std::cout << "HTTP: " << static_cast<int>(llhttp_get_http_major(parser)) << "."
              << static_cast<int>(llhttp_get_http_minor(parser)) << "\n"
              << "Method: " << llhttp_method_name(static_cast<llhttp_method_t>(llhttp_get_method(parser))) << std::endl;
    return 0;
}


int main(int argc, const char* const argv[])
{
    llhttp_t parser;
    llhttp_settings_t settings;

    llhttp_settings_init(&settings);

    settings.on_message_complete = handle_on_message_complete;

    llhttp_init(&parser, HTTP_BOTH, &settings);

    const std::string request{"GET / HTTP/1.1\r\n\r\n"};

    enum llhttp_errno err = llhttp_execute(&parser, request.c_str(), request.length());

    if (HPE_OK != err)
    {
        std::cerr << "Parse error: " << llhttp_errno_name(err) << " " << parser.reason << std::endl;
    }

    return EXIT_SUCCESS;
}
