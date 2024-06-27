#!/usr/bin/env python3

import sys
from pyroute2 import NDB


with NDB() as ndb:
    iface_ip_addr = '10.0.0.2/24'

    try:
        # Create an interface.
        with ndb.interfaces.create(ifname=sys.argv[1], kind='dummy', state='up') as interface:
            interface.add_ip(iface_ip_addr)
            # Create an address.
            interface.add_ip('10.0.0.3/24')
            interface.set('mtu', 8192)

        if iface_ip_addr in ndb.addresses:
            with ndb.addresses[iface_ip_addr] as addr:
                # Remove it.
                addr.remove()

        # ndb.routes.create(
        #    dst='192.168.100.0/24', gateway='10.0.0.1'
        # ).commit()

        # List addresses.
        for record in ndb.addresses.summary():
            print(record)
    finally:
        ndb.interfaces[sys.argv[1]].remove().commit()
