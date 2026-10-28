"""Flatten redundant shared-library symlinks before GitHub ZIP artifact upload.

Artifact upload follows symlinks, storing another full copy for every alias.
Keep the ELF SONAME used by the loader, unless a bundled ELF explicitly needs
another alias. Only operate on symlink groups within the package's lib folder.
"""

import argparse
import os
from pathlib import Path
import re
import subprocess


def dynamic_names(path, tag):
    result = subprocess.run(
        ["readelf", "--dynamic", str(path)], check=True, capture_output=True,
        text=True, env={**os.environ, "LC_ALL": "C"},
    )
    return set(re.findall(r"\(" + tag + r"\).*?\[(.*?)\]", result.stdout))


def upload_size(package):
    return sum(path.stat().st_size for path in package.rglob("*") if path.is_file())


def deduplicate(package):
    package = package.resolve(strict=True)
    library_dir = package / "lib"
    before = upload_size(package)
    needed = set()
    for path in package.rglob("*"):
        if path.is_file() and not path.is_symlink():
            with path.open("rb") as stream:
                if stream.read(4) != b"\x7fELF":
                    continue
            needed.update(dynamic_names(path, "NEEDED"))

    groups = {}
    for path in library_dir.iterdir():
        target = path.resolve(strict=True)
        if target.is_file() and target.parent == library_dir:
            groups.setdefault(target, []).append(path)

    for target, aliases in groups.items():
        if len(aliases) < 2:
            continue
        sonames = dynamic_names(target, "SONAME")
        if len(sonames) != 1:
            continue
        soname = sonames.pop()
        names = {path.name for path in aliases}
        if soname not in names or (names & needed) - {soname}:
            print(f"Keeping aliases for {target.name}: additional runtime name required")
            continue
        # All aliases resolve to this exact file; do not deduplicate unrelated
        # files merely because they happen to have identical contents.
        for alias in aliases:
            if alias != target:
                alias.unlink()
        target.rename(library_dir / soname)
        print(f"Kept {soname}; removed {len(aliases) - 1} redundant copies")

    after = upload_size(package)
    print(f"Artifact file bytes: {before:,} -> {after:,} (saved {before - after:,})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path)
    deduplicate(parser.parse_args().package)
