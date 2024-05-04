#!/usr/bin/env python3

from kafka import KafkaConsumer


consumer = KafkaConsumer('mytopic', bootstrap_servers=['localhost:9092'], group_id='my_group')
for msg in consumer:
    print(msg)
