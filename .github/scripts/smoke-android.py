"""Install the debug APK, play a touch move, and verify saved-game restoration."""

import argparse
import json
from pathlib import Path
import re
import subprocess
import time
import xml.etree.ElementTree as ET

PACKAGE = "com.mikaelfk.chess"
ACTIVITY = f"{PACKAGE}/org.qtproject.qt.android.bindings.QtActivity"
OUTPUT = Path("out/android-smoke")


def adb(*args, binary=False):
    return subprocess.run(
        ["adb", *args], check=True, capture_output=True,
        text=not binary, timeout=45,
    ).stdout


def wait_for(check, description, seconds=30):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        result = check()
        if result:
            return result
        time.sleep(0.5)
    raise RuntimeError(f"Timed out waiting for {description}")


def square_bounds(coordinate, piece):
    try:
        adb("shell", "uiautomator", "dump", "/sdcard/chess-ui.xml")
        tree = ET.fromstring(adb("shell", "cat", "/sdcard/chess-ui.xml"))
    except (subprocess.CalledProcessError, ET.ParseError):
        return None
    for node in tree.iter("node"):
        label = node.get("content-desc", "") or node.get("text", "")
        if label == f"{coordinate}, {piece}":
            return list(map(int, re.findall(r"\d+", node.get("bounds", ""))))
    return None


def tap_square(coordinate, piece):
    left, top, right, bottom = wait_for(
        lambda: square_bounds(coordinate, piece), f"accessible square {coordinate}",
    )
    adb("shell", "input", "tap", str((left + right) // 2), str((top + bottom) // 2))


def read_saved_game():
    paths = adb("shell", "run-as", PACKAGE, "find", "files", "-name", "game.json").splitlines()
    return json.loads(adb("shell", "run-as", PACKAGE, "cat", paths[0])) if paths else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path)
    args = parser.parse_args()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    adb("install", "-r", str(args.apk.resolve(strict=True)))
    adb("shell", "pm", "clear", PACKAGE)
    adb("logcat", "-c")
    try:
        adb("shell", "am", "start", "-W", "-n", ACTIVITY)
        tap_square("e2", "White Pawn")
        tap_square("e4", "empty")

        def completed_turn():
            state = read_saved_game()
            return state if state and len(state["moves"]) >= 2 else None

        recorded = wait_for(completed_turn, "saved human move and AI reply")
        require_first = recorded["moves"][0]
        if require_first[:4] != [6, 4, 4, 4]:
            raise RuntimeError(f"Touch input played the wrong move: {require_first}")
        adb("shell", "am", "force-stop", PACKAGE)
        adb("shell", "am", "start", "-W", "-n", ACTIVITY)
        wait_for(lambda: square_bounds("e4", "White Pawn"), "restored pawn on e4")
        wait_for(lambda: square_bounds("e2", "empty"), "restored empty e2 square")
        if read_saved_game() != recorded:
            raise RuntimeError("Restart changed the saved game")
        print("Android APK launches, touch input plays e2-e4, AI replies, and process restart restores the game.")
    finally:
        (OUTPUT / "logcat.txt").write_text(adb("logcat", "-d", "-t", "500"))
        (OUTPUT / "phone.png").write_bytes(adb("exec-out", "screencap", "-p", binary=True))


if __name__ == "__main__":
    main()
