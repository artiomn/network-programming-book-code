#!/usr/bin/env python3

from time import sleep, ctime
from typing import Tuple
from opcua import Client


def measure_value(m_var, mes_time: float, delay: float = 0.4) -> Tuple[float, float]:
    m_var_time = m_var.get_child('2:Time')

    while (last_mes_time := m_var_time.get_value()) <= mes_time:
        sleep(delay)

    return m_var.get_value(), last_mes_time


with Client('opc.tcp://localhost:4840') as client:
    client.connect()
    root = client.nodes.root
    multimeter = root.get_child(['Objects', '2:Multimeter'])  # or client.get_node('ns=2;i=2')
    voltmeter = multimeter.get_child('2:Voltmeter')
    ammeter = multimeter.get_child('2:Ammeter')

    print(f'Root = {root}, multimeter = {multimeter}')

    voltage = voltmeter.get_child('2:Voltage')
    mes_v_time = 0.0
    current = ammeter.get_child('2:Current')
    mes_c_time = 0.0

    while True:
        m_mode = multimeter.call_method('2:SetMode', 'VOLTAGE')
        # m_mode = multimeter.get_child('2:SetMode').set_value('VOLTAGE')
        voltage_value, mes_v_time = measure_value(voltage, mes_v_time)
        print(f'Mode = {m_mode}, value = {voltage_value} V, mes. time = {ctime(mes_v_time)}')

        m_mode = multimeter.call_method('2:SetMode', 'CURRENT')
        current_value, mes_c_time = measure_value(current, mes_c_time)

        print(f'Mode = {m_mode}, value = {current_value} A, mes. time = {ctime(mes_c_time)}')
        sleep(1)
