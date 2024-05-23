#!/usr/bin/env python3

import sys
from pyroute2 import NDB


with NDB() as ndb:
    eth = ndb.interfaces[sys.argv[1]]
    print(eth['ifname'])

    iface_ip_addr = '10.0.0.3/24'

    if iface_ip_addr in ndb.addresses:
        ndb.addresses[iface_ip_addr].remove()

    # Create an address.
    ndb.addresses.create(
        address=iface_ip_addr,
        prefixlen=24,
        index=eth['index'],
    ).commit()

    # Remove it.
    with ndb.addresses[iface_ip_addr] as addr:
        addr.remove()

    # List addresses.
    for record in ndb.addresses.summary():
        print(record)
