#!/usr/bin/env python3

import time
import paho.mqtt.client as mqtt


# pylint: disable=W0613(unused-argument)
def on_message(cln, userdata, message):
    print(f'received message: {str(message.payload.decode("utf-8"))}')


mqtt_broker = 'mqtt.eclipseprojects.io'

client = mqtt.Client('Subscriber')
client.connect(mqtt_broker)

client.loop_start()

client.on_message = on_message
client.subscribe('Example')

time.sleep(30)
client.loop_stop()
