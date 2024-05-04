#!/usr/bin/env python3

import httplib2


requester = httplib2.Http('.http_cache')
resp, content = requester.request('https://artiomsoft.ru')

# {'date': 'Tue, 31 Jan 2023 04:58:41 GMT', 'server': 'Apache', 'accept-ranges': 'bytes',
# 'content-length': '10304', 'content-type': 'text/html', 'status': '200', 'content-location': 'https://artiomsoft.ru'}
print(resp)

# b'<!DOCTYPE HTML>\n<html>\n<head>\n<meta ...
print(content)
