package com.faad2.sample.utils;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class FileManager {
    private static final String TAG = FileManager.class.getSimpleName();

    public static final String RECORDING_DIR = "recording";
    public static final String LOG_DIR = "logs";

    public static File getResourceDir(Context context, String dirName) {
        File file = getWritableFileDir(context);
        File subDirFile = new File(file, dirName);
        if (!subDirFile.exists()) {
            if (!subDirFile.mkdirs()) {
                Log.d(TAG, dirName + "directory not created");
            }
        }
        return subDirFile;
    }

    public static File getWritableFileDir(Context context) {
        if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
            File file = context.getApplicationContext().getExternalFilesDir(null);
            if (file == null) {
                file = context.getApplicationContext().getFilesDir();
            }
            return file;
        } else {
            return context.getApplicationContext().getFilesDir();
        }
    }

    public static void deleteFile(File file) {
        if (file.exists()) {
            if (!file.isDirectory()) {
                file.delete();
            } else {
                File[] files = file.listFiles();
                if (files != null) {
                    for (int i = 0; i < files.length; i++) {
                        deleteFile(files[i]);
                    }
                }
                file.delete();
            }
        }
    }

    /**
     * 将文件从assets目录，考贝到 /data/data/包名/files/ 目录中。assets 目录中的文件，会不经压缩打包至APK包中，使用时还应从apk包中导出来
     *
     * @param fileName
     */
    public static String copyAssetsFile2Phone(Context context, String fileName) {
        File file = null;
        try {
            InputStream inputStream = context.getAssets().open(fileName);
            file = new File(context.getFilesDir().getAbsolutePath() + File.separator + fileName);
            if (!file.exists() || file.length() == 0) {
                FileOutputStream fos = new FileOutputStream(file);
                int len;
                byte[] buffer = new byte[1024];
                while ((len = inputStream.read(buffer)) != -1) {
                    fos.write(buffer, 0, len);
                }
                fos.flush();
                inputStream.close();
                fos.close();
            } else {
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return file.getAbsolutePath();
    }
}
