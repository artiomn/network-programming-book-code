#!/usr/bin/env python3

from werkzeug.wrappers import Request, Response


def application(environ, start_response):
    request = Request(environ)
    text = f'Hello {request.args.get("name", "World")}!'
    response = Response(text, mimetype='text/plain')

    return response(environ, start_response)


if '__main__' == __name__:
    from werkzeug.serving import run_simple

    run_simple('127.0.0.1', 8080, application, use_debugger=True, use_reloader=True)
