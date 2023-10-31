#!/usr/bin/env python

from email.message import Message


message = Message()
message['Message-ID'] = 'msgid123'

# 1
print(len(message))

message['Message-ID'] = 'new_msgid456'

# 'msgid123'
print(message['Message-ID'])

# 2
print(len(message))

# [('Message-ID', 'msgid123'), ('Message-ID', 'new_msgid456')
print(message.items())

# True
print('MESSAGE-ID' in message)

# ['msgid123', 'new_msgid456']
print(message.get_all('message-id'))

# 'Message-ID: msgid123\nMessage-ID: new_msgid456\n\n'
print(str(message))

del message['message-ID']

# 0
print(len(message))

# None
print(message.get_all('message-id'))
