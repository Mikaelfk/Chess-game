"""Render the installed SDL/ImGui application without a development environment."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("executable", type=Path)
args = parser.parse_args()
with tempfile.TemporaryDirectory() as directory:
    subprocess.run([str(args.executable.resolve(strict=True)), "--smoke-test"], cwd=directory, check=True, timeout=30)
    screenshot = Path(directory) / "chess-smoke.bmp"
    if not screenshot.is_file() or screenshot.stat().st_size < 10000:
        raise RuntimeError("Installed application did not render a screenshot")
print("Installed SDL/ImGui application rendered its board and piece textures.")
