# Chess-game

[![Build](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml/badge.svg)](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml)

A C++ chess game for Windows, Linux, and Android. The entire frontend uses
**Dear ImGui with SDL2**. The shared `chess::Position` engine provides legal moves,
results, and computer search. No Qt SDK, libraries, QML, or plugins are needed.

## Play

The default game is White against the computer at Medium difficulty. **New game**
lets you choose your side, difficulty, or local two-player mode. Choosing Black
rotates the board and lets the computer play White's opening move.

Tap/click a piece and destination, or drag onto a highlighted square. Gold marks
selection, dots mark quiet moves, red rings mark captures (including en passant),
and red highlights a checked king. Illegal/off-board drops leave the position
unchanged. Escape cancels selection or dragging.

Tap and computer moves animate, including the rook when castling. Dragged pieces
land immediately, including promotion. Input locks during animation and computer
thinking. The computer pauses half a second before starting a worker-thread
search. New games and lifecycle changes cancel stale work. Promotion offers queen,
rook, bishop, and knight.

- **Easy:** random legal move.
- **Medium:** two-ply search with material and piece activity.
- **Hard:** up to four plies with alpha-beta pruning and a 1.5-second budget.

Material advantage and missing-piece counts appear below/above the board. Pawns
count as 1, knights/bishops as 3, rooks as 5, and queens as 9; promotions affect
balance. Checkmate, stalemate, insufficient material, threefold repetition, and
the fifty-move rule end the game. The result dialog offers **Play again**, **View
board**, and **Replay game**.

## Replay and analysis

Replay has **First**, **Previous**, **Play/Pause**, **Next**, and **Last** controls.
The evaluation bar appears **only in replay**. Its light portion represents White,
with an engine advantage in pawn units and a suggested move. Material balance is
shown while background analysis runs. Evaluation is not a win probability and
does not track repetition history.

Move pieces during replay to explore an analysis variation, including promotion.
**Return to recorded game** restores its starting point. **Close replay** returns
to the finished game. Replay and variations never change the saved game.

## Saving and input

Moves/settings are saved as version-1 JSON with atomic file replacement. Desktop
saves use SDL's per-user `Mikaelfk/Chess` application directory; Android uses
private `files/game.json`. The schema remains compatible with the earlier Android
app. Invalid saves start a new game with an error. A new game replaces the save.
Play and analysis work offline.

The shared frontend handles desktop mouse and Android touch input. Layouts scale
with display density and scroll on short screens; swipe outside the board to
scroll on a phone. System bars stay outside Android content. See the
[Android guide](Android/README.md) for APKs, builds, and mobile limitations.

## Build on Linux

Requirements: CMake **3.22+**, a C++17 compiler, Ninja or another CMake generator,
and development headers for SDL's platform backends. On Ubuntu/Debian:

```sh
sudo apt-get install build-essential cmake ninja-build libx11-dev libxext-dev \
  libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev libxss-dev
cmake --preset debug
cmake --build --preset debug --parallel 2
./out/build/debug/Chess
```

First configuration downloads hash-verified SDL 2.32.10 and Dear ImGui 1.91.9b.
JSON and PNG decoder headers are vendored. See [third_party](third_party/README.md)
for versions, licenses, and offline source overrides. Piece PNGs are embedded in
the executable, so no external asset directory is needed.

## Build on Windows

Install Visual Studio 2022 with **Desktop development with C++** and CMake. From
a Developer PowerShell:

```powershell
cmake -S . -B out/build/windows -G "Visual Studio 17 2022" -A x64
cmake --build out/build/windows --config Release --parallel 2
ctest --test-dir out/build/windows -C Release --output-on-failure
./out/build/windows/Release/Chess.exe
```

Ninja presets also work in a developer shell with `cl.exe` on PATH. SDL and ImGui
are static desktop libraries. Linux still requires system graphics and C/C++
runtimes. Windows installation includes the MSVC runtime with
`-DCHESS_DEPLOY_RUNTIME=ON`.

## Tests and line coverage

```sh
ctest --test-dir out/build/debug --output-on-failure --no-tests=error
```

Six suites cover rules/perft, sliding pieces, AI, the shared frontend controller,
actual ImGui input/layout, and SDL rendering. The renderer test uses SDL's dummy
video driver/software renderer without a desktop session. Tests exercise rotated
drag/tap input, illegal drops, promotion, replay variations/evaluation, save
validation, cancellation, and computer delays.

For GCC coverage, install Python 3.10+ and matching `gcov`:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements-coverage.txt
cmake --preset coverage
cmake --build --preset coverage --parallel 2
```

Open `out/build/coverage/coverage/index.html`, then click a filename for actual
source and execution counts. Green means covered; red means missed; yellow means
partially covered branches; unshaded lines are not executable. The self-contained
HTML works offline. Reports also include LCOV (`coverage.info`), Cobertura XML,
JSON totals, text with missed lines, and Markdown. Load LCOV in an editor viewer
such as Coverage Gutters; CI paths may need mapping to your checkout.

Reports include `ChessBackend/` and `Frontend/`, including the entry point. They
exclude libraries, generated image data, Java, Python, and tests.
**Coverage is informational; no minimum percentage is enforced.** See the
[coverage review](docs/test-coverage.md) for a migration snapshot and remaining
gaps. CI publishes **Chess-test-coverage**, retained for 30 days.

## Packages and CI

Build, coverage, and Android workflows run on PRs targeting `main` and pushes to
`main`. Desktop Build and Test coverage also allow manual runs.

Download **Chess-Windows-x64** or **Chess-Linux-x64** from a successful Build run.
Extract the ZIP, keep the `Chess` folder together, and run `Chess/bin/Chess.exe` or
`Chess/bin/Chess`. Restore Linux executable permissions with
`chmod +x Chess/bin/Chess` if necessary. Windows builds are unsigned. Linux CI
targets Ubuntu 22.04 and compatible newer environments, not all distributions.

```sh
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release -DCHESS_DEPLOY_RUNTIME=ON
cmake --build out/build/release --config Release --parallel 2
cmake --install out/build/release --config Release --prefix out/package/Chess
python .github/scripts/smoke-package.py out/package/Chess/bin/Chess
```

The package smoke test actually renders the board/textures. Linux CI runs it under
Xvfb, in addition to headless CTest rendering. Android builds ARM64/x86_64 APKs and
checks a real touch drag, AI reply, status-bar spacing, and game restoration on
an API 35 emulator.
