#!/bin/bash

TCP_HOST=google.com
TCP_PORT=80

exec 3<>"/dev/tcp/${TCP_HOST}/${TCP_PORT}"
echo -e "HEAD / HTTP/1.0\r\nHost: ${TCP_HOST}\r\nConnection: close\r\n\r\n" >&3
cat <&3

