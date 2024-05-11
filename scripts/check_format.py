#!/usr/bin/python

from pathlib import Path
import subprocess
import sys

from common.project import cpp_files


def check_format(file: Path) -> bool:
    return subprocess.run(['clang-format', '--Werror', '--dry-run',
                           '--verbose', file.as_posix()]).returncode == 0


if __name__ == '__main__':
    success: bool = True
    for file in cpp_files():
        success = success and check_format(file)
    if success:
        sys.exit(0)
    else:
        sys.exit(1)
