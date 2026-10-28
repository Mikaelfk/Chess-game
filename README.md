# Chess-game
[![CMake](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml/badge.svg)](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml)

This is bad :D


## Play against the computer

The game starts with you playing White against the computer at Medium difficulty.
Choose **Your side** (White or Black), select **Easy**, **Medium**, or **Hard**, and
click **New game** to apply your choices. When you choose Black, the computer
plays White's opening move and the board rotates so your pieces are at the bottom.
Piece selection, move hints and check highlights follow that orientation.
The selected piece's square is shaded gold, including while dragging.
Click a piece to see its legal moves: filled dots mark quiet moves, and red rings
mark captures (including en passant). Click a destination or drag the piece onto
it. Illegal drops and drops outside the board return the piece to its square;
press Escape to cancel a drag.

- **Easy:** chooses a random legal move.
- **Medium:** searches two plies ahead, balancing material and piece activity.
- **Hard:** searches up to four plies with alpha-beta pruning and a 1.5-second
  thinking budget. Search depth can be lower on slower machines.

Moves slide smoothly between squares, including both pieces during castling.
Captures disappear and promoted pawns change appearance when the move finishes.
The computer pauses briefly before each move, even on Easy difficulty. Board
input is locked during movement so turns cannot overlap.
Captured pieces are grouped by type (for example, `5x` and one pawn icon).
The side ahead in material shows a `+N` score: pawns count as 1, knights and
bishops as 3, rooks as 5, and queens as 9. The score includes promotions;
no advantage is shown when material is equal.

The computer thinks in the background. You can start a new game while it is
thinking; its old result will be discarded. Choose **Local two-player** to play
with another person instead. Pawn promotion offers all four choices. Checkmate,
stalemate, insufficient material, threefold repetition and the fifty-move rule
end the game and appear in the status bar.
A game-over dialog also shows the winner or draw reason. Choose **Play again**
to restart with the current settings, or **View board** to inspect the final position.
Choose **Replay game** in the result dialog or on the finished board to review
the game. Step through moves with **First**, **Previous**, **Next**, and **Last**,
or use **Play/Pause** for automatic playback. Replay preserves board orientation
and never changes the finished game. Starting a new game clears its replay.
The replay evaluation bar shows the built-in engine's advantage in pawn units
(material balance while searching), with a suggested move for the side to move.
This limited-depth analysis is not a guarantee of perfect play and does not track
repetition history. Click a piece and a legal destination to explore an analysis
variation, including promotion choices. **Return to recorded game** discards the
variation and restores its starting point. Navigation icons have tooltips.

The GUI and AI share a copyable `chess::Position` rules engine; search never
modifies the displayed position. Rules, sliding-piece movement, AI behaviour,
and the application are tested against that same engine.

## How to run the application

### Linux
cmake and Qt 6.3.0 or newer is required. If running older Qt version remove ```qt_standard_project_setup()``` from [CMakeLists.txt](CMakeLists.txt) and it *might* work.

Run these commands while in the project folder to run the the application
```
cmake .
make
./Chess
```


### Windows
Good luck :)

## Download a prebuilt game

The [Build workflow](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml)
builds and tests Windows x64 and Linux x64 packages on pushes to `main` and on pull
requests to `main`. It can also be started manually using **Run workflow** once
this workflow is on the default branch.

Open a successful run and download **Chess-Windows-x64** or **Chess-Linux-x64**
from **Artifacts** (GitHub sign-in is required). Artifacts are retained for 30 days.
Extract the downloaded ZIP once:

- **Windows 11:** extract `Chess-Windows-x64.zip` and open `Chess/bin/Chess.exe`.
  Keep the entire `Chess` folder together: Qt plugins, DLLs and configuration
  files are included. No compiler or Qt installation is needed. Builds are unsigned;
  Windows Smart App Control may block them.
- **Linux:** extract `Chess-Linux-x64.zip`, then run `chmod +x Chess/bin/Chess`
  and `./Chess/bin/Chess` from a graphical desktop. GitHub artifact ZIPs do not
  preserve executable permissions. Qt libraries/plugins are bundled; system
  graphics libraries are still required. The build targets Ubuntu 22.04 or compatible newer distributions,
  not every Linux distribution.

CI uses Qt 6.8.3, MSVC on Windows, and GCC on Linux. To create a runtime package
locally with Qt 6.5 or newer, configure with `-DCHESS_DEPLOY_RUNTIME=ON` and
`-DCMAKE_INSTALL_LIBDIR=lib`, build, then run
`cmake --install <build-directory> --config Release --prefix <package-directory>`.
