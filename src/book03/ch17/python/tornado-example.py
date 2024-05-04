#!/usr/bin/env python3

import asyncio
import tornado
import tornado.web


class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write('Tornado!')


def make_app():
    return tornado.web.Application(
        [
            (r'/', MainHandler),
        ]
    )


async def main():
    app = make_app()
    app.listen(8000)
    await asyncio.Event().wait()


if '__main__' == __name__:
    asyncio.run(main())
