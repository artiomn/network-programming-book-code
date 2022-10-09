#include <cstring>
#include <iostream>

extern "C"
{
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
}


void proc_exit(int signal)
{
    int wstat;
    pid_t pid;

    std::cout
        << "Signal: "
        << signal << " (" << strsignal(signal) << ")"
        << std::endl;

    while (true)
    {
        pid = waitpid(-1, &wstat, WNOHANG);
        switch (pid)
        {
            case -1:
                perror("wait()");
                return;
            case 0:
                std::cout
                    << "No waitable children"
                    << std::endl;
                return;
            default:
                std::cout
                    << "Child returns code: "
                    << wstat
                    << std::endl;
        }
    }
}


int main ()
{
    if (SIG_ERR == signal(SIGCHLD, &proc_exit))
    {
        perror("signal()");
    }

    switch (fork())
    {
        case -1:
            perror("fork()");
            return EXIT_FAILURE;
        case 0:
            std::cout
                << "Child exited..."
                << std::endl;
            return EXIT_SUCCESS;
        default:
            getchar();
    }
}
