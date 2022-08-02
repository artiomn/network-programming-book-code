#include <linux/netlink.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 4096

int main() {
    int fd, res;
    unsigned int i, len;
    char buf[BUF_SIZE];
    struct sockaddr_nl nls;

    fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (fd == -1) {
        return 1;
    }

    memset(&nls, 0, sizeof(nls));
    nls.nl_family = AF_NETLINK;
    nls.nl_pid = getpid();
    nls.nl_groups = 1;

    res = bind(fd, (struct sockaddr *)&nls, sizeof(nls));
    if (res == -1) {
        return 2;
    }

    while (1) {
        len = recv(fd, buf, sizeof(buf), 0);

        printf("============== Received %d bytes\n", len);
        for (i = 0; i < len; ++i) {
            if (buf[i] == 0) {
                printf("[0x00]\n");
            } else if (buf[i] < 33 || buf[i] > 126) {
                printf("[0x%02hhx]", buf[i]);
            } else {
                printf("%c", buf[i]);
            }
        }
        printf("<END>\n");
    }

    close(fd);
    return 0;
}
