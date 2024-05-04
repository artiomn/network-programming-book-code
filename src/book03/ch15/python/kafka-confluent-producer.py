#!/usr/bin/env python3

from confluent_kafka import Producer


p = Producer({'bootstrap.servers': 'localhost,mybroker2'})


def delivery_report(err, msg):
    if err is not None:
        print(f'Message delivery failed: {err}')
    else:
        print(f'Message delivered to {msg.topic()} [{msg.partition()}]')


for data in ['s1', 's2', 's3']:
    p.poll(0)
    p.produce('mytopic', data.encode('utf-8'), callback=delivery_report)


p.flush()
