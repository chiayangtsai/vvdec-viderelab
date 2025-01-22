# !/usr/bin/env python3

import argparse
import collections
import datetime
import json
import os
import re
import subprocess
import time
import traceback
import urllib.request
import zipfile

from common import console
from common import format
from common import platform


class ARGS:
    DEC = "dec"
    MASK = "mask"
    OUTPUT_FILE = "output_file"
    PATH = "path"
    VERBOSE = "verbose"

def parse_arguments() -> map:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dec", help="path to the decoder", type=str, required=True)
    parser.add_argument("--mask", help="test mask (regular expression)", default=".*", type=str)
    parser.add_argument("--output_file", help="file to store results", default="", type=str)
    parser.add_argument("--path", help="path to conformance tests (desc.json file)", type=str, required=True)
    parser.add_argument("--verbose", help="verbose output", default=False, action="store_true")
    args = parser.parse_args()

    ctx = {}
    ctx[ARGS.DEC] = args.dec
    ctx[ARGS.MASK] = args.mask
    ctx[ARGS.OUTPUT_FILE] = args.output_file
    ctx[ARGS.PATH] = args.path
    ctx[ARGS.VERBOSE] = args.verbose

    return ctx


def print_parameters(ctx: map) -> None:
    print(f"{format.LIGHT_WHITE}Path to the decoder{format.NC}: {ctx[ARGS.DEC]}")
    print(f"{format.LIGHT_WHITE}Test mask{format.NC}: {ctx[ARGS.MASK]}")
    print(f"{format.LIGHT_WHITE}Result output file{format.NC}: {ctx[ARGS.OUTPUT_FILE]}")
    print(f"{format.LIGHT_WHITE}Path the conformance set{format.NC}: {ctx[ARGS.PATH]}")
    print(f"{format.LIGHT_WHITE}Verbose output{format.NC}: {ctx[ARGS.VERBOSE]}")
    print("")


class DESC:
    FILENAME = "filename"
    MD5 = "md5"
    URL = "url"
    SIZE = "size"
    SKIP = "skip"


def load_desc(ctx: map) -> None:
    path = os.path.join(ctx[ARGS.PATH], "desc.json")
    with open(path) as f:
        data = json.load(f)
        for key in data:
            ctx[key] = data[key]
    if not DESC.SKIP in ctx:
        ctx[DESC.SKIP] = {}


def download_file(ctx: map) -> None:
    path = os.path.join(ctx[ARGS.PATH], ctx[DESC.FILENAME])
    if not os.path.exists(path):
        print(f"Downloading conformance file {ctx[DESC.FILENAME]} from {ctx[DESC.URL]}")
        urllib.request.urlretrieve(ctx[DESC.URL] + ctx[DESC.FILENAME], path)
    stats = os.stat(path)
    print(f"{format.LIGHT_WHITE}Conformance file{format.NC}: {path}")
    stats = os.stat(path)
    if ctx[DESC.SIZE] != stats.st_size:
        raise Exception(f"Conformance file size mismatch. Expected {ctx[DESC.SIZE]}, actual {stats.st_size}")
    print(f"{format.LIGHT_WHITE}Conformance file size{format.NC}: {stats.st_size}")
    '''
    start_time = time.time()
    with zipfile.ZipFile(path, 'r') as zip_ref:
        # Get information about all files in the zip archive
        for _ in zip_ref.infolist():
            None
    end_time = time.time()
    print(f"Time to get information about all files in the zip archive: {end_time - start_time} seconds")
    '''
    print("")


def get_file_name(path: str) -> str:
    return os.path.splitext(path)[0]


def get_compressed_path(ctx: map) -> str:
    return os.path.join(ctx[ARGS.PATH], "compressed")


def get_uncompressed_path(ctx: map) -> str:
    return os.path.join(ctx[ARGS.PATH], "uncompressed")


def uncompress(ctx: map, file: str) -> (str, str):
    file_name = get_file_name(file)
    uncompressed_path = get_uncompressed_path(ctx)
    uncompressed_file_path = os.path.join(uncompressed_path, file_name)
    bit_stream_path = os.path.join(uncompressed_file_path, f"{file_name}.bit")
    md5_path = os.path.join(uncompressed_file_path, f"{file_name}.yuv.md5")

    if os.path.exists(bit_stream_path) and os.path.exists(md5_path):
        return bit_stream_path, md5_path

    if not os.path.exists(uncompressed_path):
        os.makedirs(uncompressed_path)
    if not os.path.exists(uncompressed_file_path):
        os.makedirs(uncompressed_file_path)

    compressed_path = get_compressed_path(ctx)
    compressed_file_path = os.path.join(compressed_path, file)
    with zipfile.ZipFile(compressed_file_path, "r") as zip_ref:
        zip_ref.extractall(uncompressed_file_path)

    return bit_stream_path, md5_path


def skip_test(ctx: map, file: str) -> bool:
    file_name = get_file_name(file)
    if file_name in ctx[DESC.SKIP]:
        return True, ctx[DESC.SKIP][file_name]
    return False, None


class Result:
    passed = []
    failed = []
    skipped = []
    date = datetime.date
    time = datetime.datetime
    duration = 0


def run_tests(ctx: map) -> Result:
    result = Result()
    result.date = datetime.date.today()
    result.time = datetime.datetime.now().strftime("%H:%M:%S")

    start_time = time.time()

    decoder_path = ctx[ARGS.DEC]
    compressed_path = get_compressed_path(ctx)
    for file in os.listdir(compressed_path):
        if re.match(ctx[ARGS.MASK], file) is None:
            continue

        test_name = get_file_name(file)
        print(f"{test_name}", end='')
        skip, msg = skip_test(ctx, file)
        if skip:
            print(f"{format.LIGHT_YELLOW} skipped{format.NC}: {msg}")
            result.skipped.append(test_name)
            continue

        bitstream, md5_file = uncompress(ctx, file)

        if not os.path.exists(md5_file):
            print(f"{format.LIGHT_YELLOW} skipped{format.NC}: no valid MD5 file")
            result.skipped.append(test_name)
            continue

        with open(md5_file, "r") as f:
            md5 = f.read().strip().split()[0]

        args = [decoder_path, f"-b", f"{bitstream}", f"-md5", f"{md5}", f"-dph"]
        res = subprocess.run(args, capture_output=True, text=True)
        print(f" {format.OK}" if res.returncode == 0 else f" {format.FAILED}")
        if res.returncode != 0 and ctx[ARGS.VERBOSE]:
            print(f"OUTPUT: {res.stdout}", end='')
        result.passed.append(test_name) if res.returncode == 0 else result.failed.append(test_name)

    result.duration = time.time() - start_time

    return result


def save_to_file(ctx: map, result: Result) -> None:
    if ctx[ARGS.OUTPUT_FILE] == "":
        return

    json_result = {}
    json_result["date"] = str(result.date)
    json_result["time"] = str(result.time)
    json_result["duration"] = result.duration
    json_result["mask"] = ctx[ARGS.MASK]
    json_result["streams"] = {}

    for test in result.passed:
        json_result["streams"][test] = "passed"
    for test in result.failed:
        json_result["streams"][test] = "failed"
    for test in result.skipped:
        json_result["streams"][test] = "skipped"
    json_result["streams"] = collections.OrderedDict(sorted(json_result["streams"].items()))

    with open(ctx[ARGS.OUTPUT_FILE], "w") as f:
        json.dump(json_result, f, indent=2)


def main():
    try:
        console.clear()

        ctx = parse_arguments()
        print_parameters(ctx)
        load_desc(ctx)
        download_file(ctx)
        #unpack_file
        result = run_tests(ctx)
        save_to_file(ctx, result)

        print("")
        if len(result.passed):
            print(f"{format.LIGHT_GREEN}Passed{format.NC}: {len(result.passed)}")
        if len(result.failed):
            print(f"{format.LIGHT_RED}Failed{format.NC}: {len(result.failed)}")
        if len(result.skipped):
            print(f"{format.LIGHT_YELLOW}Skipped{format.NC}: {len(result.skipped)}")
        print("")

    except Exception as e:
        print(f"{format.ERROR}: exception caught {repr(e)}")
        if ctx[ARGS.VERBOSE]:
            traceback.print_exc()


main()
