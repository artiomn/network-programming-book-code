#!/usr/bin/env python3

import pika


connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
channel = connection.channel()

channel.queue_declare(queue='hello')

print('Sending message...')
channel.basic_publish(exchange='', routing_key='test', body='Test message!')

connection.close()
