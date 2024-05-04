#!/usr/bin/env python3

import sys
import chilkat

# This requires the Chilkat API to have been previously unlocked.
# See Global Unlock Sample for sample code

http = chilkat.CkHttp()

# Add a few custom headers.
http.SetRequestHeader('Client-ID', 'my_client_id')
http.SetRequestHeader('Client-Token', 'my_client_token')

http.put_Accept('application/json')

url = 'https://api.fiscallog.eu/sign/v1'
jsonRequestBody = '{ ... }'

# resp - CkHttpResponse.
resp = http.PostJson2(url, 'application/json', jsonRequestBody)
if not http.get_LastMethodSuccess():
    print(http.lastErrorText())
    sys.exit()

print(f'Response status code = {str(resp.get_StatusCode())}')
jsonResponseStr = resp.bodyStr()

print(jsonResponseStr)
