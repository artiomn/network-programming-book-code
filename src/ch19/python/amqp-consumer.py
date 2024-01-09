#!/usr/bin/env python3

import pika


def on_message(ch, method, properties, body):
    print(f'Message received: {body} [{ch}, {method}, {properties}]')


connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
channel = connection.channel()

channel.queue_declare(queue='test')
channel.basic_consume(queue='test', on_message_callback=on_message, auto_ack=True)

print('Consuming messages...')
channel.start_consuming()
