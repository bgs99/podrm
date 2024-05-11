#!/usr/bin/python

from pathlib import Path
import subprocess
import sys
import argparse

from common.project import cpp_files


def check_format(clang_format: str, file: Path) -> bool:
    return subprocess.run([clang_format, '--Werror', '--dry-run',
                           '--verbose', file.as_posix()]).returncode == 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument('--clang-format',
                        help='clang-format executable to use',
                        type=str,
                        default='clang-format')

    args = parser.parse_args()

    success: bool = True
    for file in cpp_files():
        success = success and check_format(args.clang_format, file)
    if success:
        sys.exit(0)
    else:
        sys.exit(1)
