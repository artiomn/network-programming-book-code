#!/usr/bin/env python3

import thriftpy2
from get_scheme_path import get_scheme_file
from thriftpy2.rpc import make_client

phones_thrift = thriftpy2.load(str(get_scheme_file()), module_name='phones_thrift')
client = make_client(phones_thrift.PhoneService, '127.0.0.1', 5000)
print(client.findAll(), client.findById(112))
