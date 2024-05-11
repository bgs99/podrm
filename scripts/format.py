#!/usr/bin/python

from pathlib import Path
import subprocess

from common.project import cpp_files


def format_file(file: Path):
    subprocess.run(['clang-format', '-i', file.as_posix()])


if __name__ == '__main__':
    for file in cpp_files():
        format_file(file)
