#!/usr/bin/env python3

import gevent
from gevent import socket


urls = ['www.google.com', 'www.example.com', 'www.python.org']

jobs = [gevent.spawn(socket.gethostbyname, url) for url in urls]

_ = gevent.joinall(jobs, timeout=2)

print([job.value for job in jobs])
