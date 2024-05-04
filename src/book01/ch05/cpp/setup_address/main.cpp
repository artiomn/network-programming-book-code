void set_ip_address()
{
    struct ifreq ifr = {0};
    struct sockaddr_in addr = {0};

    std::copy_n(ifr.ifr_name, std::min(IFNAMSIZ, strlen(ifr.ifr_name)), in.dev.device);

    addr.sin_family = AF_INET;
    int s = socket(addr.sin_family, SOCK_DGRAM, 0);
    int stat = inet_pton(addr.sin_family, in.dev.ip_addr, &addr.sin_addr);
    switch (stat)
    {
        case 0:
            raise_error("inet_pton() - invalid ip");
        case -1:
            raise_error("inet_pton() - invalid family");
        case 1: /* Ok */
            break;
        default:
            raise_error("inet_pton()");
    }

    ifr.ifr_addr = *reinterpret_cast<sockaddr *>(&addr);
    // This is just to test if address conversion happened properly.
    char buff[BUFF_SIZE];
    char *foo = inet_ntop(AF_INET, &addr.sin_addr, buff, BUFF_SIZE);
    if (!foo)
    {
        raise_error("inet_ntop()");
    }
    else
    {
        std::cout << "main = " << in.dev.ip_addr << "addr = " << buff << std::endl;
    }

    if (-1 == ioctl(s, SIOCSIFADDR, (caddr_t)&ifr)) raise_error("ioctl() - SIOCSIFADDR");
}
