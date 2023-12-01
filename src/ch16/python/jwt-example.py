#!/usr/bin/env python3

from datetime import datetime, timedelta, timezone
from pathlib import Path

from jwt import JWT, jwk_from_pem  # type: ignore[attr-defined]
from jwt.utils import get_int_from_datetime


message = {
    'iss': 'https://example.com/',
    'iat': get_int_from_datetime(datetime.now(timezone.utc)),
    'exp': get_int_from_datetime(datetime.now(timezone.utc) + timedelta(hours=1)),
}

with open(f'{Path(__file__).parent / "key.pem"}', 'rb') as fh:
    signing_key = jwk_from_pem(fh.read())

instance = JWT()
compact_jws = instance.encode(message, signing_key, alg='RS256')

print(f'JWT: {compact_jws}')

with open(f'{Path(__file__).parent / "key.pem"}', 'rb') as fh:
    verifying_key = jwk_from_pem(fh.read())
    message_received = instance.decode(compact_jws, verifying_key, do_time_check=True)

    print(f'Received JWT: {message_received}')
