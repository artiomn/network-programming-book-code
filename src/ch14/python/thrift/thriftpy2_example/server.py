#!/usr/bin/env python3

import thriftpy2
from get_scheme_path import get_scheme_file
from thriftpy2.rpc import make_server


PHONES_THRIFT = thriftpy2.load(str(get_scheme_file()), module_name='phones_thrift')


class Dispatcher:
    @staticmethod
    def findAll():
        return [
            PHONES_THRIFT.Phone(1, '38-03-23', PHONES_THRIFT.PhoneType.HOME),
            PHONES_THRIFT.Phone(2, '11-11-11', PHONES_THRIFT.PhoneType.OTHER),
        ]

    @staticmethod
    def findById(phone_id: int):
        return PHONES_THRIFT.Phone(phone_id, str(phone_id), PHONES_THRIFT.PhoneType.OTHER)


server = make_server(PHONES_THRIFT.PhoneService, Dispatcher, '127.0.0.1', 9090)
server.serve()
