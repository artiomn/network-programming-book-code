#include <iostream>
#include <sstream>
#include <string>


int main()
{
    const char delimiter = ',';

    for (std::string line; std::getline(std::cin, line);)
    {
        std::istringstream l_stream{line};
        for (std::string field; std::getline(l_stream, field, delimiter);)
            std::cout << "field: " << field << ", ";
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
