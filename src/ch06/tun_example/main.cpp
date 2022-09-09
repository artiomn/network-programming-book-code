#include <algorithm>
#include <fcntl.h>  /* O_RDWR */
#include <iostream>
#include <cassert>
#include <exception>
#include <optional>
#include <string>
#include <vector>

extern "C"
{
#include <sys/ioctl.h>
#include <unistd.h>

// Includes for struct ifreq, etc.
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
}


class TunTapNetworkInterface
{
public:
    TunTapNetworkInterface(int ni_desc) : fd_(ni_desc) {}
    TunTapNetworkInterface(TunTapNetworkInterface &&other)
    {
        fd_ = other.reset();
    }

    ~TunTapNetworkInterface()
    {
        if (-1 != fd_) close(fd_);
    }

public:
    TunTapNetworkInterface(const TunTapNetworkInterface &) = delete;
    TunTapNetworkInterface operator=(const TunTapNetworkInterface &) = delete;

public:
    operator int() const
    {
        return fd_;
    }

    int reset()
    {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

    std::string get_name() const
    {
        ifreq ifr = {0};
        if (-1 == ioctl(fd_, TUNGETIFF, &ifr))
        {
            throw std::system_error(errno, std::generic_category(), "TUNGETIFF ioctl failed");
        }

        return ifr.ifr_name;
    }

    std::string set_name(const std::string &new_name)
    {
        if (new_name.size() >= IFNAMSIZ)
        {
            throw std::logic_error("Incorrect name size");
        }

        // New socket needed, TUN/TAP descriptor can't be used.
        int sock = socket(PF_INET, SOCK_DGRAM, 0);

        if (sock < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Opening socket");
        }

        const auto old_name = std::move(get_name());

        ifreq ifr = {0};

        std::copy(old_name.begin(), old_name.end(), ifr.ifr_name);
        std::copy(new_name.begin(), new_name.end(), ifr.ifr_newname);

        if (ioctl(sock, SIOCSIFNAME, &ifr) != 0)
        {
            close(sock);
            throw std::system_error(errno, std::generic_category(), "SIOCSIFNAME ioctl failed");
        }

        return get_name();
    }
private:
    int fd_;
};


class TunTapController
{
public:
    TunTapNetworkInterface open_tap(const std::optional<std::string> &dev_name = std::nullopt)
    {
        return std::move(internal_open(false, dev_name));
    }

    TunTapNetworkInterface open_tun(const std::optional<std::string> &dev_name = std::nullopt)
    {
        return std::move(internal_open(true, dev_name));
    }

private:
    bool set_if_features(int fd, ifreq &ifr)
    {
        unsigned int features = 0;

        if (-1 == ioctl(fd, TUNGETFEATURES, &features))
        {
            return false;
        }

        if (features & IFF_ONE_QUEUE)
        {
            ifr.ifr_flags |= IFF_ONE_QUEUE;
        }

        return true;
    }

    TunTapNetworkInterface internal_open(bool open_tun, const std::optional<std::string> &dev_name)
    {
        TunTapNetworkInterface ni(open("/dev/net/tun", O_RDWR));
        ifreq ifr = {0};

        if (ni < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Opening interface");
        }

        ifr.ifr_flags = open_tun ? IFF_TUN : (IFF_TAP | IFF_NO_PI);

        if (dev_name)
        {
            if (dev_name->size() >= IFNAMSIZ)
            {
                throw std::logic_error("Incorrect name size");
            }

            std::copy(dev_name->begin(), dev_name->end(), ifr.ifr_name);
        }

        if (!set_if_features(ni, ifr))
        {
            throw std::system_error(errno, std::generic_category(), "Can't set interface features");
        }

        if (-1 == ioctl(ni, TUNSETIFF, &ifr))
        {
            throw std::system_error(errno, std::generic_category(), "TUNSETIFF ioctl failed");
        }
        return ni;
    }

};


int main(int argc, const char * const argv[])
{
    TunTapController controller;

    try
    {
        auto tt_iface = std::move((argc > 1) ? controller.open_tun(std::string(argv[1])) : controller.open_tap());

        std::cout
            << "Device " << tt_iface.get_name()
            << " was opened" << std::endl;

        tt_iface.set_name("fuc1");

        std::cout
            << "Device " << tt_iface.get_name()
            << " was renamed" << std::endl;

        std::vector<char> buf(1600);
        int bytes_count;

        while(1)
        {
            bytes_count = read(tt_iface, &buf[0], buf.size());
            std::cout
                << "Read "
                << bytes_count << " bytes "
                << "from asa0"
                << std::endl;
        }

    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

