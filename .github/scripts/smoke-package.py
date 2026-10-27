"""Check that the installed game starts without Qt's development environment."""

import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--platform", default="offscreen")
    args = parser.parse_args()
    executable = args.executable.resolve(strict=True)
    platforms = executable.parent.parent / "plugins" / "platforms"
    native_plugin = "qwindows.dll" if os.name == "nt" else "libqxcb.so"
    if not (platforms / native_plugin).is_file():
        parser.error(f"Native desktop platform plugin missing: {platforms / native_plugin}")
    environment = os.environ.copy()
    for name in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QML2_IMPORT_PATH", "LD_LIBRARY_PATH"):
        environment.pop(name, None)
    # install-qt-action adds Qt to PATH. Do not let that conceal missing DLLs.
    qt_root = environment.get("QT_ROOT_DIR") or environment.get("Qt6_DIR")
    if qt_root:
        root = Path(qt_root).resolve()
        environment["PATH"] = os.pathsep.join(
            entry for entry in environment.get("PATH", "").split(os.pathsep)
            if not Path(entry).resolve().is_relative_to(root)
        )
    environment["QT_QPA_PLATFORM"] = args.platform
    process = subprocess.Popen(
        [str(executable)], cwd=executable.parent, env=environment,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    )
    try:
        output, _ = process.communicate(timeout=5)
    except subprocess.TimeoutExpired:
        process.terminate()
        try:
            output, _ = process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            output, _ = process.communicate()
        print(output.decode(errors="replace"))
        print("Packaged game stayed in its Qt event loop for 5 seconds.")
        return 0
    print(output.decode(errors="replace"))
    print(f"Packaged game exited during startup (exit code {process.returncode}).", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
