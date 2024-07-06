#!/usr/bin/env python3

import win32api


Win32_ComputerNameDnsHostname = 1
win32api.GetComputerNameEx(Win32_ComputerNameDnsHostname)
