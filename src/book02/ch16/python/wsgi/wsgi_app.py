#!/usr/bin/env python3

from wsgi_gateway import WSGIGateway


class AppClass:
    def __init__(self, environ, start_response):
        self.environ = environ
        self.start = start_response

    def __iter__(self):
        response_headers = [('Content-type', 'text/plain')]
        self.start('200 OK', response_headers)
        yield b'Response string'


WSGIGateway().run_wsgi_app(AppClass)
