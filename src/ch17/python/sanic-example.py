#!/usr/bin/env python3

from sanic import Sanic, response


app = Sanic(__name__)


@app.route('/')
async def index(request):  # pylint: disable=W0613(unused-argument)
    return response.html(
        '<h1>Network programming!</h2>\n'
        '<form action="/welcome" method="get" id="form1">\n'
        '<label for="user">User name:</label>\n'
        '<input type="text" id="user" name="user"><br><br>\n'
        '</form>\n'
        '<button type="submit" form="form1" value="Submit">Submit</button>'
    )


@app.route('/welcome')
async def welcome(request):
    user = request.args['user'][0]
    html = '<h1>Welcome ' + user + '!</h1>'

    return response.html(html)


if '__main__' == __name__:
    app.run(host='0.0.0.0', port=8000, debug=True)
