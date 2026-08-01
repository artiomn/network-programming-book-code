#!/usr/bin/env python3

from random import randint
from urllib.parse import urlsplit
from werkzeug.routing import Map, Rule
from werkzeug.utils import redirect
from werkzeug.exceptions import HTTPException
from werkzeug.wrappers import Request, Response
from werkzeug.exceptions import NotFound


class Handler:
    url_map = Map(
        [
            Rule('/', endpoint='new_url'),
            # Rule('/<short_id>', endpoint='follow_short_link'),
            # Rule('/<short_id>+', endpoint='short_link_details')
        ]
    )

    @staticmethod
    def insert_url(url):
        return f'{url}_{randint(0, 10000)}'

    @staticmethod
    def is_valid_url(url):
        parts = urlsplit(url)
        return parts.scheme in ('http', 'https')

    def error_404(self):
        response = Response('<html><body><h1>Error 404!</h1><p>Page not found</p></body></html>', mimetype='text/html')
        response.status_code = 404

        return response

    def on_new_url(self, request):
        if 'POST' == request.method:
            url = request.form['url']
            print(f'POST query = {url}')
            if not self.is_valid_url(url):
                response = Response('Please enter a valid URL')
                response.status_code = 400
                return response

            short_id = self.insert_url(url)
            return redirect(f'/{short_id}')
        return Response('new_url.html')

    def dispatch_request(self, request):
        adapter = self.url_map.bind_to_environ(request.environ)
        try:
            endpoint, values = adapter.match()
            return getattr(self, f'on_{endpoint}')(request, **values)
        except NotFound:
            return self.error_404()
        except HTTPException as e:
            return e

    def wsgi_app(self, environ, start_response):
        request = Request(environ)
        print(type(start_response), start_response)
        return self.dispatch_request(request)(environ, start_response)

    def __call__(self, environ, start_response):
        return self.wsgi_app(environ, start_response)


if '__main__' == __name__:
    from werkzeug.serving import run_simple

    run_simple('127.0.0.1', 5000, Handler(), use_debugger=True, use_reloader=True)
