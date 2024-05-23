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
    int s = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
    kev_request key;
    key.vendor_code = KEV_VENDOR_APPLE;
    key.kev_class = KEV_NETWORK_CLASS;
    key.kev_subclass = KEV_ANY_SUBCLASS;
    int code = ioctl(s, SIOCSKEVFILT, &key);

    if (code < 0)
    {
        perror("ioctl");
        return EXIT_FAILURE;
    }

    kern_event_msg msg;

    while (true)
    {
        code = recv(s, &msg, sizeof(msg), 0);

        if (code < 0)
        {
            perror("recv");
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
    return EXIT_SUCCESS;
}
