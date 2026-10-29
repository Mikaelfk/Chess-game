"""Exercise real SDL touch input and saved-game restoration on an Android emulator."""
import argparse
import json
from pathlib import Path
import subprocess
import time

PACKAGE = 'com.mikaelfk.chess'
ACTIVITY = f'{PACKAGE}/.ChessActivity'
OUTPUT = Path('out/android-smoke')

def adb(*args, binary=False, input=None):
    return subprocess.run(['adb', *args], check=True, capture_output=True,
                          text=not binary, input=input, timeout=45).stdout

def wait_for(check, description, seconds=45):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            result = check()
            if result:
                return result
        except (subprocess.CalledProcessError, json.JSONDecodeError):
            pass
        time.sleep(.5)
    raise RuntimeError(f'Timed out waiting for {description}')

def state(name='game.json'):
    return json.loads(adb('shell', 'run-as', PACKAGE, 'cat', f'files/{name}'))


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
        left, top, right, bottom = wait_for(lambda: state('surface.json'), 'measured SDL surface')
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
        adb('shell', 'run-as', PACKAGE, 'rm', 'files/ui-layout.json', 'files/surface.json')
        adb('shell', 'am', 'start', '-W', '-n', ACTIVITY, '--ez', 'chess.smoke', 'true')
        restored = wait_for(lambda: state('ui-layout.json'), 'restored board geometry')
        if restored['moves'] != len(recorded['moves']) or restored['e4'] != 1:
            raise RuntimeError('Restart did not restore the visible pawn and move history')
        if state() != recorded:
            raise RuntimeError('Restart changed the recorded game')
        # Load a finished recording, then enter replay through the actual modal.
        adb('shell', 'am', 'force-stop', PACKAGE)
        finished = {'version': 1, 'white': True, 'computer': False, 'difficulty': 0,
                    'moves': [[6, 5, 5, 5, 5], [1, 4, 3, 4, 5],
                              [6, 6, 4, 6, 5], [0, 3, 4, 7, 5]]}
        adb('shell', 'run-as', PACKAGE, 'tee', 'files/game.json', input=json.dumps(finished))
        adb('shell', 'run-as', PACKAGE, 'rm', 'files/ui-layout.json')
        adb('shell', 'am', 'start', '-W', '-n', ACTIVITY, '--ez', 'chess.smoke', 'true')
        def replay_entry():
            current = state('ui-layout.json')
            return current if current['moves'] == 4 and current['replayEntry'][1] > 0 else None
        layout = wait_for(replay_entry, 'finished-game replay button')

        def tap(current, center):
            left, top, right, bottom = state('surface.json')
            x, y = center
            if not (0 <= x < current['width'] and 0 <= y < current['height']):
                raise RuntimeError('Replay tap target is outside the SDL surface')
            adb('shell', 'input', 'tap', str(round(left+x/current['width']*(right-left))),
                str(round(top+y/current['height']*(bottom-top))))

        tap(layout, layout['replayEntry'])
        def replay_state(step=None, playing=None, width=None):
            current = state('ui-layout.json')
            if (not current['replay'] or (step is not None and current['step'] != step)
                    or (playing is not None and current['playing'] != playing)
                    or (width is not None and current['width'] != width)):
                return None
            return current

        wait_for(replay_state, 'replay screen')
        for resolution in (None, '480x640', '640x480'):
            if resolution:
                adb('shell', 'wm', 'size', resolution)
            expected_width = int(resolution.split('x')[0]) if resolution else None
            layout = wait_for(lambda: replay_state(width=expected_width), 'resized replay screen')
            radius = layout['replayButtonSize']/2
            for x, y in layout['replayButtons']:
                if not (radius <= x <= layout['width']-radius
                        and radius <= y <= layout['height']-radius):
                    raise RuntimeError(f'Replay control is clipped at {resolution}: {layout}')
            # Real adb taps verify hit testing as well as geometry after resize.
            for button, step in ((0, 0), (3, 1), (1, 0), (4, 4)):
                tap(layout, layout['replayButtons'][button])
                layout = wait_for(lambda: replay_state(step=step), f'replay step {step}')
            tap(layout, layout['replayButtons'][2])
            layout = wait_for(lambda: replay_state(playing=True), 'replay playback')
            tap(layout, layout['replayButtons'][2])
            wait_for(lambda: replay_state(playing=False), 'replay pause')
            (OUTPUT/f'replay-{resolution or "phone"}.png').write_bytes(
                adb('exec-out', 'screencap', '-p', binary=True))
        print('SDL APK respects status bar, supports drag/restoration, and keeps replay controls visible and tappable.')
    finally:
        (OUTPUT/'logcat.txt').write_text(adb('logcat', '-d', '-t', '500'))
        (OUTPUT/'phone.png').write_bytes(adb('exec-out', 'screencap', '-p', binary=True))
        adb('shell', 'wm', 'size', 'reset')

if __name__ == '__main__':
    main()
