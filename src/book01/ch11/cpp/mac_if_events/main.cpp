extern "C"
{
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/kern_event.h>
#include <sys/socket.h>
#include <sys/types.h>
}


int main(int argc, const char* const argv[])
{
    // TODO: use SocketWrapper instead
    const int s = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
    if (s < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    kev_request key;
    key.vendor_code = KEV_VENDOR_APPLE;
    key.kev_class = KEV_NETWORK_CLASS;
    key.kev_subclass = KEV_ANY_SUBCLASS;

    if (ioctl(s, SIOCSKEVFILT, &key) < 0)
    {
        perror("ioctl");
        close(s);
        return EXIT_FAILURE;
    }

    kern_event_msg msg;

    while (true)
    {
        if (recv(s, &msg, sizeof(msg), 0) < 0)
        {
            perror("recv");
            close(s);
            return EXIT_FAILURE;
        }

        switch (msg.event_code)
        {
            case KEV_DL_IF_DETACHED:
                break;
            case KEV_DL_IF_ATTACHED:
                break;
            case KEV_DL_LINK_OFF:
                break;
            case KEV_DL_LINK_ON:
                break;
        }
    }

    close(s);
    return EXIT_SUCCESS;
}
