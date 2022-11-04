extern "C"
{
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/kern_event.h>
}


int main(int argc, const char* const argv[])
{
    // Сокет PF_SYSTEM создаётся для прослушивания событий.
    int s = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
    // Установим фильтр на события.
    kev_request key;
    key.vendor_code = KEV_VENDOR_APPLE;
    key.kev_class = KEV_NETWORK_CLASS;
    key.kev_subclass = KEV_ANY_SUBCLASS;
    int code = ioctl(s, SIOCSKEVFILT, &key);
    kern_event_msg msg;
    // Цикл получения событий.
    while (true)
    {
        // Обратите внимание, что здесь используется обычный recv().
        code = recv(s, &msg, sizeof(msg), 0);
        // Реакция на разные типы событий.
        switch(msg.event_code)
        {
           case KEV_DL_IF_DETACHED:
            // Интерфейс отсоединён.
           break;
           case KEV_DL_IF_ATTACHED:
           // Интерфейс подсоединён.
           break;
           case KEV_DL_LINK_OFF:
           // Интерфейс отключен.
           break;
           case KEV_DL_LINK_ON:
            // Интерфейс подключен.
           break;
        }
   }
   return EXIT_SUCCESS;
}
