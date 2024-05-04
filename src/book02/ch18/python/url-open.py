#!/usr/bin/env python3

from urllib.request import urlopen


with urlopen('http://artiomsoft.ru') as res:
    # 200 OK.
    print(res.code, res.msg)
    # ['Date', 'Server', 'Accept-Ranges', 'Content-Length', 'Content-Type', 'Connection']
    print(res.headers.keys())
    # 10304.
    print(res.length)
    # 200.
    print(res.status)

    # b'<!DOCTYPE HTML>\n<html>\n<head>\n<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />\n
    #  <title>Artiomsoft</title>\n<!--[if lt IE 9]>\n <script src="https://html5shiv.googlecode.com/svn/trunk/html5.js">
    # </script>\n
    # ...
    # </div>\n   </footer>\n\n</body>\n</html>\n'
    print(res.read())
