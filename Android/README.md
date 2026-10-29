# Android application

The Android app shares `ChessBackend` rules and AI with the desktop game. Its Qt
Quick interface supports tap-to-move and drag-to-move play, legal-move and capture hints, check
highlights, all three AI difficulties, either side, local two-player games, all
promotion choices, game results, and replay with automatic playback. The board
scales to the screen and the page scrolls on short or landscape displays.

Every completed move and the current settings are saved atomically in the app's
private storage. Reopening the app restores the game, repetition history, and
replay. Backgrounding the app cancels AI work; returning resumes the computer's
turn. Starting a new game replaces the saved game. Play works offline.

Drag a piece onto a highlighted square to move; illegal or off-board drops return
it to its original square. Replay includes a White/Black evaluation bar and an
engine score in pawn units, with material balance shown while analysis runs.
Evaluation is shown only during replay and is not a win probability. Analysis
uses the displayed position and does not account for repetition history. The
desktop's replay analysis variations are not part of this mobile interface.

## Install a test APK

Open the repository's **Android** GitHub Actions workflow and download
**Chess-Android-arm64-v8a** from a successful run. Extract the ZIP and install
`Chess-arm64-v8a.apk` on an ARM64 phone running **Android 9 or newer**. Allow
installation from your browser or file manager when Android requests it.
Alternatively, enable USB debugging, connect the phone, and run:

```sh
adb install -r Chess-arm64-v8a.apk
```

The **Chess-Android-x86_64** artifact is for an x86_64 emulator. These are
debug-signed test APKs, not Play Store releases. CI's debug key can change between
runs; if Android reports an incompatible signature, uninstall the previous test
app before installing the new APK (uninstalling removes its saved game). Release
distribution requires a stable signing key owned by the app publisher.

## Build locally

Install Qt **6.8.3** for Android ARM64 and the matching desktop host tools, CMake,
Ninja, JDK **17**, and the Android SDK. The workflow pins:

- SDK platform `android-35` and build tools `35.0.0`.
- NDK `26.1.10909125`.
- Minimum API 28 (Android 9), target API 35, package `com.mikaelfk.chess`.

Set `ANDROID_HOME` to your SDK directory and `QT_HOST_PATH` to the matching
desktop Qt directory. Use the Android Qt kit's `qt-cmake`, not the desktop kit:

```sh
/path/to/Qt/6.8.3/android_arm64_v8a/bin/qt-cmake -S . -B out/build/android -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DANDROID_SDK_ROOT="$ANDROID_HOME" \
  -DANDROID_NDK_ROOT="$ANDROID_HOME/ndk/26.1.10909125" \
  -DQT_HOST_PATH="$QT_HOST_PATH"
cmake --build out/build/android --target apk --parallel 2
adb install -r out/build/android/Android/android-build/ChessMobile.apk
```

Qt's Android deployment step bundles Qt libraries, QML imports, and piece images.
The Android build excludes desktop executables, CTest executables, and desktop
runtime packaging. Build and run tests using a native host kit instead.

## Preview and test the mobile interface on a desktop

With a desktop Qt 6.8+ installation containing Quick, Quick Controls, and Qt Test:

```sh
cmake -S . -B out/build/mobile -DCHESS_BUILD_MOBILE_PREVIEW=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/mobile --parallel 2
ctest --test-dir out/build/mobile --output-on-failure --no-tests=error
./out/build/mobile/Android/ChessMobile
```

`Mobile_Game_Test` covers tap selection, legal moves, board orientation, captures,
promotion and cancellation, results, repetition, replay, saved games, malformed
saves, AI replies, and background/new-game cancellation. `Mobile_UI_Test` loads
the actual QML and exercises touch input, a rotated board, settings, and narrow
and landscape layouts. The existing desktop suites run alongside these tests.

The Android workflow builds ARM64 and x86_64 APKs after host tests pass. It then
installs the x86_64 APK on an API 35 emulator, plays a touch move, waits for an AI
reply, restarts the process, and verifies game restoration. A screenshot and
device logs are uploaded as **Chess-Android-smoke-test**.
