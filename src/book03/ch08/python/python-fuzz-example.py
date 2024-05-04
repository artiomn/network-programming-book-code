#!/usr/bin/env python3

# Will be fuzzed.
from html.parser import HTMLParser

from pythonfuzz.main import PythonFuzz


# pylint: disable=no-value-for-parameter
@PythonFuzz
def fuzz(buf):
    try:
        string = buf.decode('ascii')
        parser = HTMLParser()
        parser.feed(string)
    except UnicodeDecodeError:
        pass


if __name__ == '__main__':
    # Start fuzzing.
    fuzz()
