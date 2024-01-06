#!/usr/bin/env python3

import paho.mqtt.client as mqtt


# mqtt_broker = 'mqtt.eclipseprojects.io'
mqtt_broker = 'localhost'

client = mqtt.Client('Example')
client.username_pw_set('guest', 'guest')

client.connect(mqtt_broker)

print('Publishing message')
client.publish('Example', 'hello!')
