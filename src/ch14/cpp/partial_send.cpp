extern "C"
{
#include <sys/socket.h>
#include <sys/types.h>
}

#include <exception>
#include <system_error>
#include <vector>


size_t sendall(int sock, const std::vector<char> &buffer)
{
    size_t total = 0;
    size_t bytes_left = buffer.size();

    while (total < buffer.size())
    {
        int was_sent = send(sock, buffer.data() + total, bytes_left, 0);
        if (-1 == was_sent) throw std::system_error(errno, std::system_category(), "Sending error!");
        total += was_sent;
        bytes_left -= was_sent;
    }

    return total;
}
