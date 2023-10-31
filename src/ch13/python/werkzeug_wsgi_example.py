from werkzeug.wrappers import Request, Response


def application(environ, start_response):
    request = Request(environ)
    text = f'Hello {request.args.get("name", "World")}!'
    response = Response(text, mimetype='text/plain')

    return response(environ, start_response)
