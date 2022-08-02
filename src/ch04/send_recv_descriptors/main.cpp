#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <socket_wrapper/socket_functions.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


namespace
{

const size_t buffer_size = 255;

} // namespace


static
void wyslij(int socket, int fd)  // send fd by socket
{
    struct msghdr msg = { 0 };
    std::vector<char> buf(1);
    std::vector<char> ancil_buf(CMSG_SPACE(sizeof(fd)));

    struct iovec io = { .iov_base = &buf[0], .iov_len = buf.size() };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = &ancil_buf[0];
    msg.msg_controllen = ancil_buf.size();

    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    *((int *) CMSG_DATA(cmsg)) = fd;

    msg.msg_controllen = CMSG_SPACE(sizeof(fd));

    if (sendmsg(socket, &msg, 0) < 0)
        throw std::logic_error("sendmsg");
}

static
int odbierz(int socket)  // receive fd from socket
{
    struct msghdr msg = {0};

    char m_buffer[256];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(socket, &msg, 0) < 0)
        throw std::logic_error("recvmsg");

    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);

    unsigned char * data = CMSG_DATA(cmsg);

    int fd = *((int*) data);

    return fd;
}

int main(int argc, char **argv)
{
    const char *filename = "./z7.c";

    if (argc > 1)
        filename = argv[1];
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) != 0)
        err_syserr("Failed to create Unix-domain socket pair\n");

    int pid = fork();
    if (pid > 0)  // in parent
    {
        err_remark("Parent at work\n");
        close(sv[1]);
        int sock = sv[0];

        int fd = open(filename, O_RDONLY);
        if (fd < 0)
            err_syserr("Failed to open file %s for reading\n", filename);

        wyslij(sock, fd);

        close(fd);
        nanosleep(&(struct timespec){ .tv_sec = 1, .tv_nsec = 500000000}, 0);
        err_remark("Parent exits\n");
    }
    else  // in child
    {
        err_remark("Child at play\n");
        close(sv[0]);
        int sock = sv[1];

        nanosleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 500000000}, 0);

        int fd = odbierz(sock);
        printf("Read %d!\n", fd);
        char buffer[256];
        ssize_t nbytes;
        while ((nbytes = read(fd, buffer, sizeof(buffer))) > 0)
            write(1, buffer, nbytes);
        printf("Done!\n");
        close(fd);
    }
    return 0;
}


int main(int argc, const char * const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <socket file>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;

    try
    {
        auto servinfo = socket_wrapper::get_serv_info(argv[1], SOCK_DGRAM);

        socket_wrapper::Socket sock = {servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol};

        if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            throw std::logic_error("bind");
        }

        std::vector<char> data_buff(buffer_size);
        // For the TTL.
        std::vector<char> ancil_data_buff(CMSG_SPACE(sizeof(uint32_t)));

        iovec iov[1] = { { &data_buff[0], data_buff.size() }  };

        msghdr msgh =
        {
            .msg_name = nullptr, .msg_namelen = 0,
            .msg_iov = iov, .msg_iovlen = 1,
            .msg_control = &ancil_data_buff[0],
            .msg_controllen = ancil_data_buff.size(),
            .msg_flags = 0
        };

        int recv_ttl = 1;

        // Enable IP_RECVTTL option.
        if (setsockopt(sock, IPPROTO_IP, IP_RECVTTL, &recv_ttl, sizeof(recv_ttl)) != 0)
        {
            throw std::logic_error("Set IP_RECVTTL");
        }

        // Receive auxiliary data in msgh.
        for (ssize_t n = recvmsg(sock, &msgh, 0); n; n = recvmsg(sock, &msgh, 0))
        {
            if (n < 0)
            {
                throw std::logic_error("recvmsg");
            }

            std::cout
                << n
                << " bytes was read: " << std::string(data_buff.begin(), data_buff.begin() + n)
                << std::endl;

            cmsghdr *cmsg;

            // Get ancillary data.
            for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != nullptr;
                 cmsg = CMSG_NXTHDR(&msgh, cmsg))
            {
                if (IPPROTO_IP == cmsg->cmsg_level && IP_TTL == cmsg->cmsg_type)
                {
                    uint32_t received_ttl = 0;
                    std::copy(CMSG_DATA(cmsg), CMSG_DATA(cmsg) + sizeof(received_ttl), &received_ttl);

                    std::cout
                        << "IP_RECVTTL was received: " << received_ttl
                        << std::endl;
                    break;
                }
            }

            if (nullptr == cmsg)
            {
                // Error: IP_TTL was not enabled or small buffer or I/O error.
                throw std::logic_error("IP_RECVTTL receiving");
            }

            break;
        }

    }
    catch (const std::logic_error &e)
    {
        std::cerr
            << e.what()
            << ": " << sock_wrap.get_last_error_string() << "!"
            << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

