#!/usr/bin/env python3

import atheris


# Decorator may be used.
with atheris.instrument_imports():
    from html.parser import HTMLParser
    import sys


def TestOneInput(data):
    try:
        string = data.decode('ascii')
        parser = HTMLParser()
        parser.feed(string)
    except UnicodeDecodeError:
        pass


atheris.Setup(sys.argv, TestOneInput)

# Start fuzzing.
atheris.Fuzz()
