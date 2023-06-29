#!/usr/bin/python

import win32api


def GetUserName():
    try:
        return win32api.GetUserName()
    except win32api.error:
        # Seeing 'access denied' errors here for non-local users (presumably
        # without permission to login locally).  Get the fully-qualified
        # username, although a side-effect of these permission-denied errors
        # is a lack of Python codecs - so printing the Unicode value fails.
        # So just return the repr(), and avoid codecs completely.
        return repr(win32api.GetUserNameEx(win32api.NameSamCompatible))
