import uwsgi


# pylint: disable=W0613(unused-argument)
def application(env, start_response):
    uwsgi.websocket_handshake(env['HTTP_SEC_WEBSOCKET_KEY'], env.get('HTTP_ORIGIN', ''))

    while True:
        msg = uwsgi.websocket_recv()
        uwsgi.websocket_send(msg)
