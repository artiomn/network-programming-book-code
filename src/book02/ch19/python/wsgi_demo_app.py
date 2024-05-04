#!/usr/bin/env python3


def demo_app(environ, start_response):
    # pylint: disable=C0415(import-outside-toplevel)
    from io import StringIO

    stdout = StringIO()

    print('Hello world!', file=stdout)
    print(file=stdout)
    h = sorted(environ.items())

    for k, v in h:
        print(k, '=', repr(v), file=stdout)

    start_response('200 OK', [('Content-Type', 'text/plain; charset=utf-8')])

    return [stdout.getvalue().encode('utf-8')]
