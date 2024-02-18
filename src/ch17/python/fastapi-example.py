#!/usr/bin/env python3

from typing import Union
from fastapi import FastAPI


app = FastAPI()


@app.get('/')
def read_root():
    return {'Hello': 'World'}


@app.get('/items/{item_id}')
def read_item(item_id: int, value: Union[str, None] = None):
    return {'item_id': item_id, 'value': value}


if '__main__' == __name__:
    import uvicorn

    config = uvicorn.Config(f'{__name__}:app', port=8000, log_level='info')
    server = uvicorn.Server(config)
    server.run()
