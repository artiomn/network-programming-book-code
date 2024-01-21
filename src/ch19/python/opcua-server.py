#!/usr/bin/env python3

import asyncio
from enum import Enum
from time import time
import random
from asyncua import Server, uamethod, ua


class Multimeter:
    class Mode(Enum):
        VOLTAGE = 0
        CURRENT = 1

    def __init__(self):
        self._mode: 'Multimeter.Mode' = self.Mode.VOLTAGE

    @property
    def mode(self) -> 'Multimeter.Mode':
        return self._mode

    @mode.setter
    def mode(self, mode: 'Multimeter.Mode'):
        if isinstance(mode, str):
            mode = Multimeter.Mode[mode]
        elif not isinstance(mode, self.Mode):
            raise TypeError(f'Mode type is incorrect [{type(mode)}]!')
        self._mode = mode

    def measure(self) -> float:
        if self.Mode.VOLTAGE == self.mode:
            return random.uniform(310.0, 380.0)
        if self.Mode.CURRENT == self.mode:
            return random.uniform(5.0, 30.0)

        raise NotImplementedError(f'Unknown mode {self.mode}!')


class MMSwitcher:
    def __init__(self, device):
        self._device = device

    @uamethod
    def __call__(self, parent, d_mode):
        self._device.mode = d_mode
        print(f'New device mode = {self._device.mode}')
        return d_mode


async def setup_multimeter(server, device):
    ns = await server.register_namespace('Multimeter')
    objects = server.get_objects_node()

    t_enums = await server.get_root_node().get_child(['0:Types', '0:DataTypes', '0:BaseDataType', '0:Enumeration'])
    mode_enum_type = await t_enums.add_data_type(ns, 'MultimeterMode')
    es = await mode_enum_type.add_property(
        0, 'EnumStrings', [ua.LocalizedText(value.name) for value in Multimeter.Mode]
    )
    await es.add_property(0, 'ValueRank', 1)
    await es.add_property(0, 'ArrayDimensions', 1)

    multimeter = await objects.add_object(ns, 'Multimeter')
    await multimeter.add_property(ns, 'device_sn', '123-231')

    mode = await multimeter.add_variable(ns, 'Mode', device.mode, datatype=mode_enum_type.nodeid)
    await mode.set_writable()

    await multimeter.add_method(ns, 'SetMode', MMSwitcher(device), [mode_enum_type.nodeid], [mode_enum_type.nodeid])

    voltmeter = await multimeter.add_object(ns, 'Voltmeter')
    ammeter = await multimeter.add_object(ns, 'Ammeter')

    vm_voltage = await voltmeter.add_variable(ns, 'Voltage', 0.0)
    vm_mes_time = await voltmeter.add_variable(ns, 'Time', time())

    am_current = await ammeter.add_variable(ns, 'Current', 0.0)
    am_mes_time = await ammeter.add_variable(ns, 'Time', time())

    return vm_voltage, vm_mes_time, am_current, am_mes_time


async def main(server_url='opc.tcp://0.0.0.0:4840'):
    multimeter_device = Multimeter()

    server = Server()
    await server.init()
    server.set_endpoint(server_url)
    server.set_server_name('Test server')

    async with server:
        vm_voltage, vm_mes_time, am_current, am_mes_time = await setup_multimeter(server, multimeter_device)

        while True:
            # Obtain information from the device.
            print(f'Device mode = {multimeter_device.mode}')
            if Multimeter.Mode.VOLTAGE == multimeter_device.mode:
                voltage = multimeter_device.measure()
                print(f'Setting voltage = {voltage}')
                await vm_voltage.set_value(voltage)
                await vm_mes_time.set_value(time())
            elif Multimeter.Mode.CURRENT == multimeter_device.mode:
                current = multimeter_device.measure()
                print(f'Setting current = {current}')
                await am_current.set_value(current)
                await am_mes_time.set_value(time())

            # Measured values.
            mes_voltage = await vm_voltage.get_value()
            mes_current = await am_current.get_value()

            print(f'Mes. voltage: {mes_voltage} V\nMes. current: {mes_current} A')
            await asyncio.sleep(0.5)


asyncio.run(main())
