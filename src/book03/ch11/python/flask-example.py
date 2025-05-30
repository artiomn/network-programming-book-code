#!/usr/bin/env python3

from flask import Flask


app = Flask(__name__)


@app.route('/')
def index():
    return '''<!DOCTYPE html>
<html>
<head>
    <title>Network programming book</title>
</head>
<body>
<h1>Network programming</h1>
</body>
</html>'''


@app.route('/about')
def about():
    return '<h1>Welcome flask!</h1>'


if '__main__' == __name__:
    app.run(debug=True)  # nosec B201:flask_debug_true
