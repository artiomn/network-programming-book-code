extern "C"
{
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include <cstring>
#include <iostream>


void proc_exit(int signal)
{
    std::cout << "Signal: " << signal << " (" << strsignal(signal) << ")" << std::endl;

    while (true)
    {
        int wstat = 0;
        const pid_t pid = waitpid(-1, &wstat, WNOHANG);
        switch (pid)
        {
            case -1:
                perror("wait()");
                return;
            case 0:
                std::cout << "No waitable children" << std::endl;
                return;
            default:
                std::cout << "Child returns code: " << wstat << std::endl;
        }
    }
}


int main()
{
    if (SIG_ERR == signal(SIGCHLD, &proc_exit))
    {
        perror("signal()");
        return EXIT_FAILURE;
    }

    switch (fork())
    {
        case -1:
            perror("fork()");
            return EXIT_FAILURE;
        case 0:
            std::cout << "Child exited..." << std::endl;
            return EXIT_SUCCESS;
        default:
            getchar();
    }
    return EXIT_SUCCESS;
}
