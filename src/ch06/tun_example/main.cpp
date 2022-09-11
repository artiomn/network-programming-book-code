#include <algorithm>
#include <iostream>
#include <cassert>
#include <exception>
#include <optional>
#include <string>
#include <vector>

extern "C"
{
// O_RDWR.
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Includes for struct ifreq, etc.
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

// Interface types.
#include <linux/if_arp.h>
}


const auto buffer_size = 1600;


int perform_ioctl(int fd, const int call_id, void *result, const std::string &msg)
{
    auto ioctl_res = ioctl(fd, call_id, result);
    if (-1 == ioctl_res)
    {
        throw std::system_error(errno, std::generic_category(), msg);
    }

    return ioctl_res;
}



class TunTapNetworkInterface
{
public:
    static const auto mac_size = 6;

public:
    TunTapNetworkInterface(int ni_desc) :
        fd_(ni_desc)
    {
    }

    TunTapNetworkInterface(TunTapNetworkInterface &&other)
    {
        fd_ = other.reset();
    }

    virtual ~TunTapNetworkInterface()
    {
        if (-1 != fd_) close(fd_);
    }

    virtual const std::string type() const = 0;

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
        auto ifr{std::move(get_iff())};

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

    void set_interface_type(unsigned int type = ARPHRD_INFINIBAND)
    {
        perform_ioctl(fd_, TUNSETLINK, &type, "TUNSETLINK ioctl failed");
    }

protected:
    ifreq get_iff() const
    {
        ifreq ifr = {0};
        perform_ioctl(fd_, TUNGETIFF, &ifr, "TUNGETIFF ioctl failed");

        return ifr;
    }

private:
    int fd_;
};


class Tun : public TunTapNetworkInterface
{
public:
    Tun(int ni_desc, ifreq &ifr) :
        TunTapNetworkInterface::TunTapNetworkInterface(ni_desc)
    {
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

        if (!set_if_features(ifr))
        {
            throw std::system_error(errno, std::generic_category(), "Can't set interface features");
        }

        perform_ioctl(*this, TUNSETIFF, &ifr, "TUNSETIFF ioctl failed");
    }

    virtual const std::string type() const override { return "tun"; }

protected:
    bool set_if_features(ifreq &ifr)
    {
        unsigned int features = 0;

        if (-1 == ioctl(*this, TUNGETFEATURES, &features))
        {
            return false;
        }

        if (features & IFF_ONE_QUEUE)
        {
            ifr.ifr_flags |= IFF_ONE_QUEUE;
        }

        return true;
    }


};


class Tap : public TunTapNetworkInterface
{
public:
    Tap(int ni_desc, ifreq &ifr) :
        TunTapNetworkInterface::TunTapNetworkInterface(ni_desc)
    {
        ifr.ifr_flags = IFF_TAP;
        perform_ioctl(*this, TUNSETIFF, &ifr, "TUNSETIFF ioctl failed");
    }

    virtual const std::string type() const override { return "tap"; }

public:
    void set_hw_addr(const uint8_t hw_addr[TunTapNetworkInterface::mac_size])
    {
        ifreq ifr{std::move(get_iff())};

        int sock = socket(AF_INET, SOCK_DGRAM, 0);

        std::copy(hw_addr, hw_addr + TunTapNetworkInterface::mac_size, &ifr.ifr_hwaddr.sa_data[0]);

        if (sock < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Can't create socket");
        }

        // Only for Ethernet.
        ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

        if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Can't set interface address");
        }
    }

};


class TunTapController
{
public:
    Tap open_tap(const std::optional<std::string> &dev_name = std::nullopt)
    {
        return std::move(internal_open<Tap>(dev_name));
    }

    Tun open_tun(const std::optional<std::string> &dev_name = std::nullopt)
    {
        return std::move(internal_open<Tun>(dev_name));
    }

private:
    template<class T>
    T internal_open(const std::optional<std::string> &dev_name)
    {
        int dev_fd = open("/dev/net/tun", O_RDWR);

        if (dev_fd < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Opening interface");
        }

        ifreq ifr = {0};

        if (dev_name)
        {
            if (dev_name->size() >= IFNAMSIZ)
            {
                throw std::logic_error("Incorrect name size");
            }

            std::copy(dev_name->begin(), dev_name->end(), ifr.ifr_name);
        }

        return T(dev_fd, ifr);
    }

};


int main(int argc, const char * const argv[])
{
    TunTapController controller;

    try
    {
        auto tt_iface = std::move((argc > 1) ? controller.open_tap(std::string(argv[1])) : controller.open_tap());
        auto dev_name = std::move(tt_iface.get_name());

        std::cout
            << "Device " << dev_name
            << " was opened." << std::endl;

        std::vector<char> buf(buffer_size);
        int bytes_count;

        uint8_t mac_addr[] = {0x12, 0x10, 0x20, 0x30, 0x40, 0x50};

        tt_iface.set_hw_addr(mac_addr);

        while (true)
        {
            std::cout
                << "Waiting for a data..."
                << std::endl;
            bytes_count = read(tt_iface, &buf[0], buf.size());
            std::cout
                << "Read "
                << bytes_count << " bytes "
                << "from " << dev_name << "."
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

