#!/usr/bin/env python3

from time import strftime, sleep, gmtime
from opcua import Client


URL = "opc.tcp://localhost:4840"

with Client(URL) as client:
    client.connect()
    root = client.nodes.root
    multimeter = root.get_child(['Objects', '2:Multimeter'])  # or client.get_node('ns=2;i=2')
    voltmeter = multimeter.get_child('2:Voltmeter')
    ammeter = multimeter.get_child('2:Ammeter')

    print(f'Root = {root}, multimeter = {multimeter}')

    while True:
        m_mode = multimeter.call_method('2:SetMode', 'VOLTAGE')
        voltage = voltmeter.get_child('2:Voltage').get_value()
        mes_v_time = voltmeter.get_child('2:Time').get_value()
        disp_v_time = strftime('%Y/%M/%D %H:%M:%S', gmtime(mes_v_time))
        print(f'Mode = {m_mode}, value = {voltage} V, mes. time = {disp_v_time}')

        m_mode = multimeter.call_method('2:SetMode', 'CURRENT')

        current = ammeter.get_child('2:Current').get_value()
        mes_a_time = voltmeter.get_child('2:Time').get_value()
        disp_a_time = strftime('%Y/%M/%D %H:%M:%S', gmtime(mes_a_time))
        print(f'Mode = {m_mode}, value = {current} A, mes. time = {disp_a_time}')
        sleep(1)
