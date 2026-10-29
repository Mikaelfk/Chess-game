package com.mikaelfk.chess;

import org.libsdl.app.SDLActivity;

/** SDL owns native rendering and touch/lifecycle event dispatch. */
public final class ChessActivity extends SDLActivity {
    @Override protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }
    @Override protected String[] getArguments() {
        // Debug emulator tests use actual rendered geometry to drive adb touches.
        return getIntent().getBooleanExtra("chess.smoke", false)
            ? new String[] { "--android-smoke" } : new String[0];
    }
}
