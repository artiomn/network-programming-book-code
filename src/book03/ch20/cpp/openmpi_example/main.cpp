#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <mpi.h>


int main(int argc, const char* const argv[])
{
    // Initialize MPI environment.
    MPI_Init(&argc, const_cast<char***>(&argv));

    // Number of processes.
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // The rank of the process.
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Gets the name of the processor.
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Until this point, all programs have been doing exactly the same.
    // Here, we check the rank to distinguish the roles of the programs.
    if (0 == world_rank)
    {
        int other_rank;

        std::cout
            << "Hello world from processor \""
            << processor_name
            << "\" rank " << world_rank
            << " out of " << world_size << " processors"
            << std::endl;

        // Send messages to all other processes.
        for (other_rank = 1; other_rank < world_size; ++other_rank)
        {
            std::stringstream buf;
            buf
                << "Hello "
                << other_rank
                << " from "
                << world_rank
                << "!";
            MPI_Send(buf.str().c_str(), buf.str().size(), MPI_CHAR, other_rank,
                     0, MPI_COMM_WORLD);
        }

        // Receive messages from all other processes.
        for (other_rank = 1; other_rank < world_size; other_rank++)
        {
            std::vector<char> buf;
            buf.resize(256);
            MPI_Recv(buf.data(), buf.size(), MPI_CHAR, other_rank,
                     0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout
                << "Message: " << std::string(buf.begin(), buf.end())
                << std::endl;
        }

    }
    else
    {
        std::vector<char> buf;
        buf.resize(256);
        // Receive message from process #0.
        MPI_Recv(buf.data(), buf.size(), MPI_CHAR, 0,
                 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::cout
            << "Process " << world_rank
            << " received data: " << std::string(buf.begin(), buf.end())
            << std::endl;

        // Send message to process #0.
        std::stringstream sendss;
        sendss
            << "Process " << world_rank
            << " reporting for duty.";
        MPI_Send(sendss.str().c_str(), sendss.str().size(), MPI_CHAR, 0,
                 0, MPI_COMM_WORLD);

    }

    // Finish MPI environment.
    MPI_Finalize();
}
