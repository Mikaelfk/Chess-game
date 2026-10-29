# Android application

Android uses the same Dear ImGui frontend, SDL renderer, rules, and AI as desktop.
`ChessActivity` extends SDLActivity to dispatch lifecycle and touch events to C++.
There is no Qt or separate mobile UI/controller.

Play supports tap/drag, legal hints, either side, three AI difficulties, local
games, four promotion choices, and replay variations. Evaluation and suggested
moves appear only in replay. Captured-piece icons sit on the capturing player's
side and rotate with the board. Tap/computer moves animate; drops land immediately.
Backgrounding cancels work; returning resumes the computer's turn. Saves use
private `files/game.json` and the previous version-1 schema. Replay never
changes the recorded save.

Replay keeps its icon toolbar fixed at the bottom: first, previous, play/pause,
next, last. Close (X) is at the top, and the curved arrow beside the move count
returns from a variation to the recording. Navigation stays visible in portrait
and landscape; the board scales to the available height.

Layouts scale with display density. Swipe outside the board to scroll on short
or landscape screens. System bars stay visible; the Android 15 theme opts out
of enforced edge-to-edge at target SDK 35. A future SDK 36 target must replace
that opt-out with explicit SDL safe-area handling.

## Install

Download **Chess-Android-arm64-v8a** from a successful **Android** workflow run,
extract the ZIP, and install `Chess-arm64-v8a.apk` on an Android 9+ ARM64 phone.
Allow installation from the browser/file manager when prompted. With USB debugging:

```sh
adb install -r Chess-arm64-v8a.apk
```

**Chess-Android-x86_64** is for an emulator. These are debug-signed test APKs.
An incompatible CI signature requires uninstalling the old app, which removes
its save. Releases need a stable publisher-owned key. The application ID stays
`com.mikaelfk.chess`.

## Build

Install JDK **17**, Android command-line tools, and these SDK components:

```sh
sdkmanager 'platforms;android-35' 'build-tools;35.0.0' 'cmake;3.22.1' 'ndk;26.1.10909125'
export ANDROID_HOME=/path/to/android-sdk
cd Android
./gradlew assembleDebug -PchessAbis=arm64-v8a
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

Use `gradlew.bat` on Windows. `chessAbis` defaults to `arm64-v8a`; use `x86_64`
for an emulator. Gradle is pinned to 8.11.1, Android Gradle Plugin to 8.9.2,
minimum SDK to 28, and target SDK to 35. First builds need network access for
Gradle, Android artifacts, SDL, and ImGui; play needs none.

CMake builds shared `SDL2` and `main` libraries and embeds piece images. SDL is
fetched to `out/deps/SDL`; Gradle compiles SDLActivity's Java sources from there
after native configuration. Only host builds include CTest executables. See the
[main guide](../README.md) for shared controller/input tests and coverage.

## CI smoke test

The workflow builds ARM64/x86_64 APKs. An API 35 emulator checks that the SDL
surface starts below the status bar, a real `adb input swipe` plays e2-e4, the
computer replies, and force-stop/relaunch preserve recorded moves/settings.

ImGui draws one native surface rather than Android View widgets. The debug smoke
launch (`--ez chess.smoke true`) writes actual board geometry to private
`ui-layout.json`; tests map it through measured `surface.json` bounds to drive real touches.
Normal launches do not write this file. Screenshots/logcat are uploaded as
**Chess-Android-smoke-test**.

ImGui does not expose the former Qt accessibility tree. Screen-reader access to
individual squares is a known migration limitation. Host tests do not replace
APK validation on physical devices and other Android versions.
