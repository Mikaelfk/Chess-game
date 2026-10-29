# Test coverage review

The ImGui migration replaces the Qt desktop/mobile implementations and their
framework-specific tests. Old file/line percentages do not describe this branch.

Measured with GCC 14.2, gcovr 8.4, and a Debug build after migration: **687/726
production lines covered (94.6%)**, 72/73 functions (98.6%), and 57.6% branches.
All six CTest suites passed with and without instrumentation. This is a snapshot;
regenerate it using [the main coverage instructions](../README.md#tests-and-line-coverage).
**Coverage is informational, with no enforced minimum.**

## Source viewer

Open `out/build/coverage/coverage/index.html`, select a filename, and inspect
its source, line numbers, and execution counts. Green lines are covered, red
lines missed, yellow lines partially branch-covered, and unshaded lines are not
executable. This self-contained HTML works offline. LCOV, Cobertura XML, JSON,
Markdown, and text reports accompany it. CI publishes **Chess-test-coverage**;
extract it before opening the HTML. Editor LCOV viewers may need CI path mapping.

The report includes `ChessBackend/` and `Frontend/`, including application startup
and SDL rendering. It excludes SDL/ImGui, vendored headers, generated PNG byte
arrays, tests, Android Java, and Python build/smoke scripts.

## Regression tests

- `position_test`: legal moves/perft, special moves, terminal outcomes, draw
  boundaries, repetition identity, pinned en passant, and minor-piece material.
- `sliding_piece_test`: blocker, edge, capture, and check interactions.
- `ai_test`: difficulties, seeded randomness, cancelled/terminal searches.
- `frontend_test`: tap/drag selection, legal hints, illegal drops, capture material,
  all promotion choices/cancellation, history-based captured-piece counts (including
  en passant, promotion, and replay variations), saved-game validation and atomic replacement,
  computer delays, background/new-game cancellation, repetition, replay analysis,
  stale results, branch restoration, and preserving the recorded game.
- `imgui_test`: actual ImGui event injection, drag/tap input in both orientations,
  animation policy, off-board mapping, capture-icon geometry for both orientations,
  high-DPI texture levels, phone/landscape/desktop layouts, pinned replay toolbar
  geometry, and actual replay navigation/restore/close through icon controls.
- `renderer_smoke`: the real executable renders embedded pieces through SDL's
  software renderer and dummy video backend, without a display server.

## Remaining gaps and platform checks

The principal uncovered C++ paths include some settings/result buttons,
error messages, low-level SDL initialization failures, lifecycle events arriving
from the operating system, and animated castling rendering. The controller and
ImGui tests exercise a subset of the interface; line coverage is not a statement
that all GUI interactions or engine branches are correct.

Desktop CI additionally runs the installed renderer under Xvfb on Linux and on
Windows. Android CI builds APKs and drives an API 35 emulator to check real touch,
status-bar spacing, AI replies, and saved-game restoration. Those checks are not
included in the native C++ coverage snapshot. Physical-device behavior, display
cutouts, screen readers, and Android versions other than API 35 still need device
validation. ImGui currently lacks the former per-square accessibility tree.
