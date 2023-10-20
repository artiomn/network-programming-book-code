async def app(scope, receive, send):
    if scope['type'] != 'http':
        raise AssertionError(f'Protocol "{scope["type"]}" is not supported!')

    print(receive)

    await send(
        {
            'type': 'http.response.start',
            'status': 200,
            'headers': [
                [b'content-type', b'text/plain'],
            ],
        }
    )

    await send(
        {
            'type': 'http.response.body',
            'body': f'ASGI test message. Method: "{scope["method"]}".'.encode(),
        }
    )
