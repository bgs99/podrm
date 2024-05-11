from pathlib import Path
from typing import List, Generator

root: Path = Path(__file__).parent.parent.parent

source_code_dirs: List[Path] = [root /
                                dir for dir in ['include', 'lib', 'test']]


def cpp_files() -> Generator[Path, None, None]:
    for dir in source_code_dirs:
        yield from dir.rglob('*.cpp')
        yield from dir.rglob('*.hpp')
