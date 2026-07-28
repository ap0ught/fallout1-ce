package com.alexbatalov.falloutce;

import android.content.Intent;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

import java.io.File;

public class MainActivity extends SDLActivity {
    private boolean noExit = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final File externalFilesDir = getExternalFilesDir(null);

        final File configFile = new File(externalFilesDir, getString(R.string.config_file_name));
        if (!configFile.exists()) {
            final File masterDatFile = new File(externalFilesDir, getString(R.string.master_dat_name));
            final File critterDatFile = new File(externalFilesDir, getString(R.string.critter_dat_name));
            if (!masterDatFile.exists() || !critterDatFile.exists()) {
                final Intent intent = new Intent(this, ImportActivity.class);
                startActivity(intent);

                noExit = true;
                finish();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (!noExit) {
            // Needed to make sure libc calls exit handlers, which releases
            // in-game resources.
            System.exit(0);
        }
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            getString(R.string.lib_name),
        };
    }
}
