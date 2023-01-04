#include <boost/mpi.hpp>
#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>


namespace mpi = boost::mpi;


int main()
{
    mpi::environment env;
    mpi::communicator world;

    if (0 == world.rank())
    {
        world.send(1, 0, std::string("Hello"));
        std::string msg;
        world.recv(1, 1, msg);
        std::cout
            << msg << "!"
            << std::endl;
    }
    else
    {
        std::string msg;
        world.recv(0, 0, msg);
        std::cout
            << msg << ", ";
        std::cout.flush();
        world.send(0, 1, std::string("world"));
    }

    return EXIT_SUCCESS;
}
