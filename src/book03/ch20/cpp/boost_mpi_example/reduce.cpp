#include <cstdlib>
#include <iostream>

#include <boost/mpi.hpp>


namespace mpi = boost::mpi;


int main()
{
    mpi::environment env;
    mpi::communicator world;

    std::srand(time(0) + world.rank());

    int my_number = std::rand();

    std::cout << "  Process " << world.rank() << " value = " << my_number << std::endl;

    if (0 == world.rank())
    {
        int minimum;
        reduce(world, my_number, minimum, mpi::minimum<int>(), 0);
        std::cout << "\nThe minimum value is " << minimum << std::endl;
    }
    else
    {
        reduce(world, my_number, mpi::minimum<int>(), 0);
    }

    return EXIT_SUCCESS;
}
