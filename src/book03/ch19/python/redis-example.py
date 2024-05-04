#!/usr/bin/env python3

import redis


r = redis.Redis(host='localhost', port=6379, db=0)

r.publish('user_data', 'User Message!')

sub = r.pubsub()
sub.subscribe('user_data')

while True:
    msg = sub.get_message()
    print(f'Message data: {msg["data"]}')
