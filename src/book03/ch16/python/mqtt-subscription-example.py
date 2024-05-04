#!/usr/bin/env python3

import time
import paho.mqtt.client as mqtt


# pylint: disable=W0613(unused-argument)
def on_message(cln, userdata, message):
    print(f'received message: {str(message.payload.decode("utf-8"))}')


# mqtt_broker = 'mqtt.eclipseprojects.io'
mqtt_broker = 'localhost'

client = mqtt.Client('Subscriber')
client.on_message = on_message

client.username_pw_set('guest', 'guest')

client.connect(mqtt_broker)

client.loop_start()

print('Subscription...')
client.subscribe('Example')

time.sleep(30)
client.loop_stop()
