extern "C"
{
// O_RDWR.
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Includes for struct ifreq, etc.
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/types.h>

// Interface types.
#include <linux/if_arp.h>
}

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>


int perform_ioctl(int fd, int call_id, void *result, const std::string &msg)
{
    const auto ioctl_res = ioctl(fd, call_id, result);
    if (-1 == ioctl_res)
    {
        throw std::system_error(errno, std::system_category(), msg);
    }

    return ioctl_res;
}


class TunTapNetworkInterface
{
public:
    static constexpr auto mac_size = 6;

public:
    explicit TunTapNetworkInterface(int ni_desc) : fd_(ni_desc) {}

    TunTapNetworkInterface(TunTapNetworkInterface &&other) { fd_ = other.reset(); }

    TunTapNetworkInterface &operator=(TunTapNetworkInterface &&other)
    {
        fd_ = other.reset();
        return *this;
    }

    virtual ~TunTapNetworkInterface()
    {
        if (-1 != fd_) close(fd_);
    }

    virtual std::string type() const = 0;

public:
    TunTapNetworkInterface(const TunTapNetworkInterface &) = delete;
    TunTapNetworkInterface &operator=(const TunTapNetworkInterface &) = delete;

public:
    operator int() const { return fd_; }

    int reset()
    {
        const int fd = fd_;
        fd_ = -1;
        return fd;
    }

    std::string get_name() const { return get_iff().ifr_name; }

    std::string set_name(const std::string &new_name)
    {
        if (new_name.size() >= IFNAMSIZ)
        {
            throw std::logic_error("Incorrect name size");
        }

        // New socket needed, TUN/TAP descriptor can't be used.
        const int sock = socket(PF_INET, SOCK_DGRAM, 0);

        if (sock < 0)
        {
            throw std::system_error(errno, std::system_category(), "Opening socket");
        }

        const auto old_name = get_name();

        ifreq ifr = {0};

        std::copy(old_name.begin(), old_name.end(), ifr.ifr_name);
        std::copy(new_name.begin(), new_name.end(), ifr.ifr_newname);

        try
        {
            perform_ioctl(sock, SIOCSIFNAME, &ifr, "SIOCSIFNAME failed");
        }
        catch (...)
        {
            close(sock);
            throw;
        }

        close(sock);

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
    explicit Tun(int ni_desc, ifreq &ifr) : TunTapNetworkInterface::TunTapNetworkInterface(ni_desc)
    {
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

        if (!set_if_features(ifr))
        {
            throw std::system_error(errno, std::system_category(), "Can't set interface features");
        }

        perform_ioctl(*this, TUNSETIFF, &ifr, "TUNSETIFF ioctl failed");
    }
    Tun(Tun &&) = default;

    std::string type() const override { return "tun"; }

    Tun &operator=(const Tun &) = delete;
    Tun &operator=(Tun &&) = default;

protected:
    bool set_if_features(ifreq &ifr) noexcept
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
    explicit Tap(int ni_desc, ifreq &ifr) : TunTapNetworkInterface::TunTapNetworkInterface(ni_desc)
    {
        ifr.ifr_flags = IFF_TAP;
        perform_ioctl(*this, TUNSETIFF, &ifr, "TUNSETIFF ioctl failed");
    }
    Tap(Tap &&) = default;

    std::string type() const override { return "tap"; }

    Tap &operator=(Tap &&) = default;
    Tap &operator=(const Tap &) = delete;

public:
    void set_hw_addr(const uint8_t hw_addr[TunTapNetworkInterface::mac_size])
    {
        ifreq ifr = get_iff();
        std::copy(hw_addr, hw_addr + TunTapNetworkInterface::mac_size, &ifr.ifr_hwaddr.sa_data[0]);

        const int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            throw std::system_error(errno, std::generic_category(), "Can't create socket");
        }

        // Only for Ethernet.
        ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
        try
        {
            perform_ioctl(sock, SIOCSIFHWADDR, &ifr, "Can't set interface address");
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            close(sock);
            throw;
        }
        close(sock);
    }
};


class TunTapController
{
public:
    using DevName = std::optional<std::string>;

    Tap open_tap(const DevName &dev_name = std::nullopt) const { return internal_open<Tap>(dev_name); }

    Tun open_tun(const DevName &dev_name = std::nullopt) const { return internal_open<Tun>(dev_name); }

private:
    template <class T>
    T internal_open(const DevName &dev_name) const
    {
        const int dev_fd = open("/dev/net/tun", O_RDWR);

        if (dev_fd < 0)
        {
            throw std::system_error(errno, std::system_category(), "Opening interface");
        }

        ifreq ifr = {0};

        if (dev_name.has_value())
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


int main(int argc, const char *const argv[])
{
    constexpr auto buffer_size = 1600;
    const TunTapController controller;

    try
    {
        Tap tt_iface{(argc > 1) ? controller.open_tap(std::string(argv[1])) : controller.open_tap()};
        auto &&dev_name = tt_iface.get_name();

        std::cout << "Device " << dev_name << " was opened." << std::endl;

        std::array<char, buffer_size> buf;

        const uint8_t mac_addr[] = {0x12, 0x10, 0x20, 0x30, 0x40, 0x50};

        tt_iface.set_hw_addr(mac_addr);

        while (true)
        {
            std::cout << "Waiting for data..." << std::endl;
            const int bytes_count = read(tt_iface, &buf[0], buf.size());
            std::cout << "Read " << bytes_count << " bytes "
                      << "from " << dev_name << "." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
