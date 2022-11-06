package com.faad2.audio.converter;

import android.util.Log;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class Faad2AudioConverter implements Library {
    private static final String TAG = "Faad2AudioConverter";

    private static Faad2AudioConverter sInstance = null;

    public static final String JNA_LIBRARY_NAME = "audioconverter";

    static {
        Native.register(Faad2AudioConverter.class, Faad2AudioConverter.JNA_LIBRARY_NAME);
    }

    private Faad2AudioConverter() {
        Log.w(TAG, "Faad2AudioConverter");
    }

    public static Faad2AudioConverter getInstance() {
        if (sInstance == null) {
            sInstance = new Faad2AudioConverter();
        }
        return sInstance;
    }

    public int convertAac2Wav(String aacFilePath, String wavFilePath, String outMonoWav, int sampleRate) {
        Log.w(TAG, "B convertAac2Wav!");
        int ret = convertAac2MonoWav(aacFilePath, wavFilePath, outMonoWav, sampleRate);
        Log.w(TAG, "E convertAac2Wav!" + ret);
        return ret;
    }

    public native int convertAac2MonoWav(String sourceAac, String destWave, String outMonoWav, int sampleRate);
}
