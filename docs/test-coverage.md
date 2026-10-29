# Test coverage review

Measured with GCC 14.2, gcovr 8.4, and a Debug coverage build. All four CTest
suites pass with and without instrumentation. These numbers are a snapshot;
regenerate reports using the [coverage instructions](../README.md#tests-and-line-coverage).

**Coverage is informational, with no minimum percentage enforced locally or in CI.**
Current result: **834/856 lines (97.4%)**. Before the additional regression tests,
the same build covered 833/856 lines (97.3%).

## Browse covered and uncovered code

Run the `coverage` build target and open `out/build/coverage/coverage/index.html`
in a browser. Select any filename to view its complete source with numbered lines:
green means covered, yellow means executed with some branches missed, red means
uncovered, and unshaded lines are non-executable.
The execution-count column shows how often each executable line ran. The HTML
file contains all source files and styles and works offline on its own.

In GitHub Actions, use the **Download the source coverage report** link in the
run summary, extract the ZIP, and open `index.html`. The artifact also contains
`coverage.info` for displaying the same coverage in an LCOV-compatible editor.

| Production file | Covered / total lines | Coverage | Missed lines |
| --- | ---: | ---: | --- |
| `ChessApplication/ChessApplication.cpp` | 618 / 632 | 97.8% | 36, 95, 97, 139–140, 158–159, 725–728, 759–761 |
| `ChessApplication/ChessApplication.h` | 6 / 6 | 100% | None |
| `ChessApplication/main.cpp` | 0 / 6 | 0% | 5–10 |
| `ChessBackend/ChessAI.cpp` | 68 / 68 | 100% | None |
| `ChessBackend/Position.cpp` | 140 / 142 | 98.6% | 59, 187 |
| `ChessBackend/Position.h` | 2 / 2 | 100% | None |

## Additional tests

High line coverage still left behavior worth checking explicitly. Added tests
exercise single-minor and bishop/knight draw combinations, the 99-to-100 halfmove
boundary, clock resets after pawn moves and captures, checkmate taking precedence
over the fifty-move rule, and repetition identity with different turns, castling
rights, legal en-passant captures, and pinned en-passant captures.

AI tests now check every difficulty on checkmate, stalemate, insufficient material,
and fifty-move draws, pre-cancelled searches, and repeatable seeded Easy moves.
They verify that terminal and cancelled searches produce no move, evaluation,
search depth, or visited nodes.

## Remaining gaps

Replay castling animation and promotion analysis are useful next GUI regression
tests. The remaining GUI lines also include defensive fallbacks for invalid
assets, missing animation items, and absent AI moves. The missing-king rule guard
and a compiler-attributed closing brace remain uncovered in `Position.cpp`.
The application entry point is included as uncovered because CTest launches the
test application; the existing package smoke test exercises executable startup
separately without contributing to this report.

Reported branch coverage is 61.9%; no coverage percentage is enforced. Branch
counts include compiler-generated C++/Qt paths and should be reviewed separately.
The HTML report provides per-line execution counts and branch details. Neither
metric replaces assertions about correct behavior.
