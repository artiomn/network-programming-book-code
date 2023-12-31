#!/usr/bin/env python3

import paho.mqtt.client as mqtt


mqtt_broker = 'mqtt.eclipseprojects.io'

client = mqtt.Client('Example')
client.connect(mqtt_broker)

client.publish('Example', 'hello!')
