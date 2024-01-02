#!/usr/bin/env python3

from kafka import KafkaConsumer


consumer = KafkaConsumer('metadata.broker.list', group_id='my_favorite_group')
for msg in consumer:
    print(msg)
