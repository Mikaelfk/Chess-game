"""Exercise real SDL touch input and saved-game restoration on an Android emulator."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import time
import xml.etree.ElementTree as ET

PACKAGE = 'com.mikaelfk.chess'
ACTIVITY = f'{PACKAGE}/.ChessActivity'
OUTPUT = Path('out/android-smoke')

def adb(*args, binary=False):
    return subprocess.run(['adb', *args], check=True, capture_output=True, text=not binary, timeout=45).stdout

def wait_for(check, description, seconds=45):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            result = check()
            if result:
                return result
        except (subprocess.CalledProcessError, json.JSONDecodeError, ET.ParseError):
            pass
        time.sleep(.5)
    raise RuntimeError(f'Timed out waiting for {description}')

def state(name='game.json'):
    return json.loads(adb('shell', 'run-as', PACKAGE, 'cat', f'files/{name}'))

def surface():
    adb('shell', 'uiautomator', 'dump', '/sdcard/chess-ui.xml')
    root = ET.fromstring(adb('shell', 'cat', '/sdcard/chess-ui.xml'))
    for node in root.iter('node'):
        if node.get('class') == 'android.view.SurfaceView' and node.get('package') == PACKAGE:
            return list(map(int, re.findall(r'\d+', node.get('bounds', ''))))
    return None

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('apk', type=Path)
    args = parser.parse_args()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    adb('install', '-r', str(args.apk.resolve(strict=True)))
    adb('shell', 'pm', 'clear', PACKAGE)
    adb('logcat', '-c')
    try:
        adb('shell', 'am', 'start', '-W', '-n', ACTIVITY, '--ez', 'chess.smoke', 'true')
        layout = wait_for(lambda: state('ui-layout.json'), 'rendered board geometry')
        left, top, right, bottom = wait_for(surface, 'SDL surface')
        if top <= 0:
            raise RuntimeError('SDL surface overlaps the status bar')
        def point(name):
            x, y = layout[name]
            return str(round(left+x/layout['width']*(right-left))), str(round(top+y/layout['height']*(bottom-top)))
        adb('shell', 'input', 'swipe', *point('from'), *point('to'), '400')
        def completed():
            saved = state()
            return saved if len(saved['moves']) >= 2 else None
        recorded = wait_for(completed, 'human drag and computer reply')
        if recorded['moves'][0][:4] != [6, 4, 4, 4]:
            raise RuntimeError('Drag played the wrong move')
        adb('shell', 'am', 'force-stop', PACKAGE)
        adb('shell', 'run-as', PACKAGE, 'rm', 'files/ui-layout.json')
        adb('shell', 'am', 'start', '-W', '-n', ACTIVITY, '--ez', 'chess.smoke', 'true')
        restored = wait_for(lambda: state('ui-layout.json'), 'restored board geometry')
        if restored['moves'] != len(recorded['moves']) or restored['e4'] != 1:
            raise RuntimeError('Restart did not restore the visible pawn and move history')
        if state() != recorded:
            raise RuntimeError('Restart changed the recorded game')
        print('SDL APK renders below status bar, accepts drag, replies, and restores its game.')
    finally:
        (OUTPUT/'logcat.txt').write_text(adb('logcat', '-d', '-t', '500'))
        (OUTPUT/'phone.png').write_bytes(adb('exec-out', 'screencap', '-p', binary=True))

if __name__ == '__main__':
    main()
