# !/usr/bin/env python3

import importlib
import os
import sys

from . import format


def exit_on_error(message: str, log=None) -> None:
    #if None != log:
    #    log.log(log.ERROR, message)
    print("\n" + format.ERROR + ": " + message)
    sys.exit(-1)


def expand_path(path: str) -> str:
    path = os.path.expandvars(path)
    path = os.path.expanduser(path)
    path = os.path.abspath(path)
    return path


def load_module(path: str, log=None):
    path = expand_path(path)
    dir = os.path.dirname(path)
    filename = os.path.basename(path)
    filename = filename if not os.path.splitext(filename)[1] == ".py" else \
        os.path.splitext(filename)[0]
    sys.path.append(dir)
    try:
        module = importlib.import_module(filename)
    except:
        return None
    return module
