package com.hybroad.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.text.TextUtils;
import android.util.Log;

public class IOUtitls {
  private final static String TAG = "--IOUtitls--";
  private static ExecutorService sThreadPool = Executors.newFixedThreadPool(2);

  public static void copyFile(final String sourceFilePath,
      final String targetDirectory, final String targetTitle) {
    if (null == sThreadPool || TextUtils.isEmpty(sourceFilePath)
        || TextUtils.isEmpty(targetDirectory) || TextUtils.isEmpty(targetTitle)) {
      Log.e(TAG, "invalid parameter");
      return;
    }

    Log.i(TAG, "Start copy file..." + sourceFilePath);
    sThreadPool.execute(new Runnable() {
      @Override
      public void run() {
        /* Check target directory */
        File targetDirectoryFile = new File(targetDirectory);
        if (!targetDirectoryFile.exists() || !targetDirectoryFile.isDirectory()) {
          /*
           * Layer by layer to create the parent directory which does not exist,
           * and set readable and writable
           */
          int index = -1;
          while ((index = targetDirectory.indexOf(File.separator, index)) > -1) {
            File file = new File(targetDirectory.substring(0, index + 1));
            if (!file.exists() || !file.isDirectory()) {
              boolean isSuccessfull = file.mkdir();
              if (!isSuccessfull) {
                Log.e(TAG, "Create necessary directory " + file.toString()
                    + " failure!");
                return;
              }
              file.setReadable(true, false);
              file.setWritable(true, false);
              file.setExecutable(true, false);
            }
            ++index;
          }

          /* Finally create the target directory, and set readable and writable */
          boolean isSuccessfull = targetDirectoryFile.mkdir();
          if (!isSuccessfull) {
            Log.e(TAG,
                "Create necessary directory " + targetDirectoryFile.toString()
                    + " failure!");
            return;
          }
          targetDirectoryFile.setReadable(true, false);
          targetDirectoryFile.setWritable(true, false);
          targetDirectoryFile.setExecutable(true, false);
        }

        /* Copy file */
        FileInputStream fileInputStream = null;
        InputStream internetInputStream = null;
        FileOutputStream fileOutputStream = null;
        HttpURLConnection httpURLConnection = null;
        URL url = null;
        int position;
        if ((position = sourceFilePath.lastIndexOf('.')) < 0)
          return;
        String suffix = sourceFilePath.substring(position);
        File saveFile = new File(targetDirectory + File.separator + targetTitle
            + suffix);

        try {
          fileOutputStream = new FileOutputStream(saveFile);
          if (sourceFilePath.startsWith("http://")) {
            url = new URL(sourceFilePath);
            httpURLConnection = (HttpURLConnection) url.openConnection();
            httpURLConnection.setConnectTimeout(5000);
            httpURLConnection.setReadTimeout(5000);
            httpURLConnection.setRequestMethod("GET");
            int responseCode;
            if (200 == (responseCode = httpURLConnection.getResponseCode())) {
              internetInputStream = httpURLConnection.getInputStream();
            } else {
              Log.e(TAG, "connect failure! responseCode: " + responseCode);
              if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
              }
              return;
            }

            byte[] buffer = new byte[4096];
            int len;
            fileOutputStream = new FileOutputStream(saveFile);
            while ((len = internetInputStream.read(buffer)) != -1) {
              fileOutputStream.write(buffer, 0, len);
            }
          } else {
            fileInputStream = new FileInputStream(sourceFilePath);
            byte[] buffer = new byte[4096];
            int len;
            while ((len = fileInputStream.read(buffer)) != -1) {
              fileOutputStream.write(buffer, 0, len);
            }
          }
        } catch (FileNotFoundException e) {
          Log.e(TAG, "Can't find file ", e);
          return;
        } catch (IOException e) {
          Log.e(TAG, "IO operater error ", e);
          return;
        } catch (Exception e) {
          Log.e(TAG, "Other Exception ", e);
          return;
        } finally {
          try {
            if (sourceFilePath.startsWith("http://")) {
              if (null != internetInputStream)
                internetInputStream.close();
              if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
              }
            } else {
              if (null != fileInputStream)
                fileInputStream.close();
            }
            if (null != fileOutputStream)
              fileOutputStream.close();
          } catch (IOException e) {
            e.printStackTrace();
          }
        }

        /* Finally set read and write permissions */
        saveFile.setReadable(true, false);
        saveFile.setWritable(true, false);
        Log.i(TAG, "Copy file ok!");
      }
    });
  }

  /* Warning!!! */
  public static void deleteFileOrFolder(final String targetFilePath) {
    if (null == sThreadPool || TextUtils.isEmpty(targetFilePath)
        || targetFilePath.equals("/mnt/nand")) {
      Log.e(TAG, "invalid parameter");
      return;
    }

    sThreadPool.execute(new Runnable() {
      @Override
      public void run() {
        File file = new File(targetFilePath);
        Log.i(TAG, "Start delete file " + file.toString());
        if (!file.exists()) {
          Log.i(TAG, "Target file does not exist: " + file.toString());
          return;
        }
        if (file.isFile())
          file.delete();
        else
          deleteFolder(targetFilePath);
        Log.i(TAG, "Delete file ok: " + file.toString());
      }

      private void deleteFolder(String filePath) {
        if (!filePath.endsWith(File.separator)) {
          filePath = filePath + File.separator;
          File dirFile = new File(filePath);

          if (!dirFile.exists() || !dirFile.isDirectory()) {
            return;
          }

          /* Delete subfile */
          File[] files = dirFile.listFiles();
          for (File file : files) {
            if (file.isFile())
              file.delete();
            else
              deleteFolder(file.getAbsolutePath());
          }

          /* Delete self */
          dirFile.delete();
        }
      }
    });
  }
}
