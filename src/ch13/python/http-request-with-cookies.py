#!/usr/bin/env python3

import http.cookiejar
import urllib.request
from pathlib import Path


cookies_file = Path('cookies.txt')

cj = http.cookiejar.MozillaCookieJar()

if cookies_file.exists():
    cj.load(str(cookies_file))

opener = urllib.request.build_opener(urllib.request.HTTPCookieProcessor(cj))
r = opener.open('http://yandex.ru/')

print(r.msg)
cj.save(str(cookies_file))

print(cj)
