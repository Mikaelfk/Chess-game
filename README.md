# Chess-game
[![CMake](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml/badge.svg)](https://github.com/Mikaelfk/Chess-game/actions/workflows/cmake.yml)

This is bad :D


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
builds and tests Windows x64 and Linux x64 packages on every push and on pull
requests to `main`. It can also be started manually using **Run workflow** once
this workflow is on the default branch.

Open a successful run and download **Chess-Windows-x64** or **Chess-Linux-x64**
from **Artifacts** (GitHub sign-in is required). Artifacts are retained for 30 days.
Extract the downloaded artifact, then extract the archive inside it:

- **Windows 11:** extract `Chess-Windows-x64.zip` and open `Chess/bin/Chess.exe`.
  Keep the entire `Chess` folder together: Qt plugins, DLLs and configuration
  files are included. No compiler or Qt installation is needed. Builds are unsigned;
  Windows Smart App Control may block them.
- **Linux:** extract `Chess-Linux-x64.tar.gz` and run `./Chess/bin/Chess` from a
  graphical desktop. The tar archive preserves executable permissions and library
  symlinks. Qt libraries/plugins are bundled; system graphics libraries are still
  required. The build targets Ubuntu 22.04 or compatible newer distributions,
  not every Linux distribution.

CI uses Qt 6.8.3, MSVC on Windows, and GCC on Linux. To create a runtime package
locally with Qt 6.5 or newer, configure with `-DCHESS_DEPLOY_RUNTIME=ON` and
`-DCMAKE_INSTALL_LIBDIR=lib`, build, then run
`cmake --install <build-directory> --config Release --prefix <package-directory>`.
