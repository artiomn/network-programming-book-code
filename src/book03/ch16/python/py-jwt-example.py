#!/usr/bin/env python3

import sys

sys.path.insert(0, '/usr/lib/python3/dist-packages')

# pylint: disable=wrong-import-position
import jwt  # noqa


encoded_jwt = jwt.encode({'some': 'payload'}, 'secret', algorithm='HS256')
print(encoded_jwt)
jwt.decode(encoded_jwt, 'secret', algorithms=['HS256'])
