#!/usr/bin/env python3

import requests


resp = requests.get('https://artiomsoft.ru')

# <Response [200]>
print(resp)

# b'<!DOCTYPE HTML>\n<html>\n<head>\n<me ...
print(resp.content)
