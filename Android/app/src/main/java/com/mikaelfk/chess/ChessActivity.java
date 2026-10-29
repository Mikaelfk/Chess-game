package com.mikaelfk.chess;

import android.os.Bundle;
import android.util.Log;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import org.libsdl.app.SDLActivity;

/** SDL owns native rendering and touch/lifecycle event dispatch. */
public final class ChessActivity extends SDLActivity {
    @Override protected String[] getLibraries() {
        return new String[] { "SDL2", "main" };
    }
    @Override protected String[] getArguments() {
        return getIntent().getBooleanExtra("chess.smoke", false)
            ? new String[] { "--android-smoke" } : new String[0];
    }
    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getIntent().getBooleanExtra("chess.smoke", false) && mSurface != null) {
            // A native SurfaceView need not appear in the accessibility tree.
            // Record measured screen coordinates after layout for adb touch tests.
            mSurface.addOnLayoutChangeListener((view, l, t, r, b, ol, ot, oright, ob) -> {
                int[] origin = new int[2];
                view.getLocationOnScreen(origin);
                String data = "[" + origin[0] + "," + origin[1] + ","
                    + (origin[0] + view.getWidth()) + "," + (origin[1] + view.getHeight()) + "]";
                try (FileOutputStream output = new FileOutputStream(new File(getFilesDir(), "surface.json"))) {
                    output.write(data.getBytes(StandardCharsets.UTF_8));
                } catch (java.io.IOException error) {
                    Log.e("Chess", "Could not record smoke-test surface bounds", error);
                }
            });
        }
    }
}
