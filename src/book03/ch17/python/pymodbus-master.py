#!/usr/bin/env python3

import pymodbus.client as modbusClient


with modbusClient.ModbusTcpClient(
    host='localhost',
    port=1502,
    # Common optional parameters:
    # framer='binary',
    timeout=1,
    #    retries=3,
    #    retry_on_empty=False,y
    #    close_comm_on_error=False,
    #    strict=True,
    # TCP setup parameters
    #    source_address=('localhost', 0),
) as client:
    client.connect()
    regs = client.read_holding_registers(10, 5)

    if regs:
        print(f'Regs = {regs}, regs[2] = {regs.registers[2]}')
    else:
        print('Unable to read!')
