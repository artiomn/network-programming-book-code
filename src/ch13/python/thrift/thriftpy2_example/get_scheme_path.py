from pathlib import Path


def get_scheme_file() -> Path:
    scheme_file = Path(__file__)

    for _ in range(4):
        scheme_file = scheme_file.parent

    scheme_file = scheme_file / 'cpp' / 'thrift' / 'phones.thrift'

    return scheme_file.resolve()
