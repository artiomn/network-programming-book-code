#!/usr/bin/python

"""Spawn multiple workers and wait for them to complete."""

import gevent
from gevent import monkey

# Patches stdlib (including socket and ssl modules) to cooperate with other greenlets.
monkey.patch_all()

# pylint: disable=wrong-import-position
# flake8: noqa: C0413
import requests


urls = ['https://www.google.com/', 'https://www.yandex.ru/', 'https://www.python.org/']


def print_head(url):
    print(f'Starting {url}')
    data = requests.get(url).text
    print(f'{url}: {len(data)} bytes: {data[:30]!r}')


jobs = [gevent.spawn(print_head, _url) for _url in urls]
gevent.wait(jobs)
