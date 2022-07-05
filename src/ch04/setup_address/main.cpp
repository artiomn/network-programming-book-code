void setip (int fd)
{
     struct ifreq ifr;
     struct sockaddr_in addr;
     int stat, s;

     memset(&ifr, 0, sizeof(ifr));
     memset(&addr, 0, sizeof(addr));
     strncpy(ifr.ifr_name, in.dev.device, IFNAMSIZ);

     addr.sin_family = AF_INET;
     s = socket(addr.sin_family, SOCK_DGRAM, 0);
     stat = inet_pton(addr.sin_family, in.dev.ip_addr, &addr.sin_addr);
     if (stat == 0)
         raise_error("inet_pton() - invalid ip");
     if (stat == -1)
         raise_error("inet_pton() - invalid family");
     
     if (stat == 1);
     else
         raise_error("inet_pton()");
     
     ifr.ifr_addr = *(struct sockaddr *) &addr;
     /* This is just to test if address conversion happened properly */
     char buff[BUFF_SIZE];
     char * foo;
     foo = inet_ntop(AF_INET, &addr.sin_addr, buff, BUFF_SIZE);
     if (foo == NULL)
         raise_error("inet_ntop()");
     else
         printf("main = %s, addr = %s\n",in.dev.ip_addr, buff);
 
     //if (ioctl(fd, SIOCSIFADDR, (caddr_t) &ifr) == -1)
     if (ioctl(s, SIOCSIFADDR, (caddr_t) &ifr) == -1)
         raise_error("ioctl() - SIOCSIFADDR");
 }

