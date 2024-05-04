import os
import sys


class WSGIGateway:
    enc = sys.getfilesystemencoding()

    def __init__(self):
        self.environ = {
            'wsgi.input': sys.stdin.buffer,
            'wsgi.errors': sys.stderr,
            'wsgi.version': (1, 0),
            'wsgi.multithread': False,
            'wsgi.multiprocess': True,
            'wsgi.run_once': True,
        }
        self._headers_set = []
        self._headers_sent = []

    def run_wsgi_app(self, application):
        """Run WSGI app."""

        environ = self.environ
        self.environ.update({k: self._unicode_to_wsgi(v) for k, v in os.environ.items()})

        if environ.get('HTTPS', 'off') in ('on', '1'):
            environ['wsgi.url_scheme'] = 'https'
        else:
            environ['wsgi.url_scheme'] = 'http'

        self._headers_set = []
        self._headers_sent = []

        result = application(environ, self._start_response)

        try:
            for data in result:
                if data:
                    self._write(data)
            if not self._headers_sent:
                self._write('')
        finally:
            if hasattr(result, 'close'):
                result.close()

    def _write(self, data):
        out = sys.stdout.buffer

        if not self._headers_set:
            raise AssertionError('write() before start_response()')

        # pylint: disable=unbalanced-tuple-unpacking
        status, response_headers = self._headers_sent[:] = self._headers_set
        out.write(self._wsgi_to_bytes(f'Status: {status}\r\n'))

        for header_name, header_value in response_headers:
            out.write(self._wsgi_to_bytes(f'{header_name}: {header_value}\r\n'))

        out.write(self._wsgi_to_bytes('\r\n'))

        out.write(data)
        out.flush()

    def _start_response(self, status, response_headers, exc_info=None):
        """WSGI application handler."""
        if exc_info:
            try:
                if self._headers_sent:
                    raise exc_info[1].with_traceback(exc_info[2])
            finally:
                exc_info = None
        elif self._headers_set:
            raise AssertionError('Headers already set!')

        self._headers_set[:] = [status, response_headers]

        return self._write

    def _unicode_to_wsgi(self, u):
        """Convert value into the WSGI "bytes-as-unicode" string."""
        return u.encode(self.enc, 'surrogateescape').decode('iso-8859-1')

    @staticmethod
    def _wsgi_to_bytes(s):
        return s.encode('iso-8859-1')
