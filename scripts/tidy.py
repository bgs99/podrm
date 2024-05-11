#!/usr/bin/python

from pathlib import Path
import subprocess
import argparse

from common.project import cpp_files


if __name__ == '__main__':
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument('--clang-format',
                        help='clang-format executable to use',
                        type=str,
                        default='clang-format')
    parser.add_argument('--compile-commands',
                        help='Path to the directory with compile commands',
                        type=Path,
                        default='build')

    args = parser.parse_args()

    subprocess.run(['run-clang-tidy', '-quiet', '-p', args.compile_commands.as_posix(),
                    *[file.as_posix() for file in cpp_files()]])
