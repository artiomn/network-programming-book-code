#!/usr/bin/env python3

from kafka import KafkaProducer


bootstrap_servers = ['localhost:9092']
topic_name = 'metadata.broker.list'
producer = KafkaProducer(bootstrap_servers=bootstrap_servers)

for data in ['s1', 's2', 's3']:
    ack = producer.send(topic_name, data.encode('utf-8'))
    metadata = ack.get()

    print(metadata.topic)
    print(metadata.partition)
