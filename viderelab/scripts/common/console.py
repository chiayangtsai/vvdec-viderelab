# !/usr/bin/env python3

import os
import sys


def clear() -> None:
    if 'win32' == sys.platform:
        _ = os.system('cls')
    else:
        _ = os.system('clear')
