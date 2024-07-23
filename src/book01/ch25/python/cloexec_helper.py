#!/usr/bin/env python3

import fcntl


def set_close_exec(fd):
    """
    Helper to set CLOEXEC.

    :fd: int file descriptor
    """
    flags = fcntl.fcntl(fd, fcntl.F_GETFD)
    fcntl.fcntl(fd, fcntl.F_SETFD, flags | fcntl.FD_CLOEXEC)
