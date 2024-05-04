#!/usr/bin/env python3

import urllib3


http = urllib3.PoolManager()
r = http.request('GET', 'http://httpbin.org/robots.txt', timeout=2.0)

# 200
print(r.status)

# b'User-agent: *\nDisallow: /deny\n'
print(r.data)
