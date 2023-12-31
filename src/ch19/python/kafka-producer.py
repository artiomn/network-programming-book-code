#!/usr/bin/env python3

from kafka import KafkaProducer


bootstrap_servers = ['localhost:9092']
topicName = 'metadata.broker.list'
producer = KafkaProducer(bootstrap_servers=bootstrap_servers)
producer = KafkaProducer()
ack = producer.send(topicName, b'hey there!')
metadata = ack.get()
print(metadata.topic)
print(metadata.partition)
