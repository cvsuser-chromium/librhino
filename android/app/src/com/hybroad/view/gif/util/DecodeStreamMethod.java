package com.hybroad.view.gif.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

public class DecodeStreamMethod extends DecodeMethod {
  private static final String TAG = "DecodeStreamMethod";
  private URL urlObj;
  private InputStream mInputStream;

  public DecodeStreamMethod(URL urlObj, int targetWidth, int targetHeight) {
    this.urlObj = urlObj;
    this.targetWidth = targetWidth;
    this.targetHeight = targetHeight;
  }

  public DecodeStreamMethod(InputStream inputStream, int targetWidth,
      int tragetHeight) {

    this.mInputStream = inputStream;
    this.targetWidth = targetWidth;
    this.targetHeight = tragetHeight;
    Log.d(TAG, "mInputStream = " + this.mInputStream + ", targetWidth = "
        + this.targetWidth + ", this.targetHeight = " + this.targetHeight);
  }

  @Override
  public void prepare() throws DecodeException {
    HttpURLConnection conn = null;
    InputStream inputStream = null;
    // try {
    // conn =
    // (HttpURLConnection) this.urlObj.openConnection(); //
    // conn.setReadTimeout(8000);
    // conn.setConnectTimeout(8000);
    try {
      Log.d(TAG, "urlObj = " + urlObj + ", " + urlObj.getPath().toString());
      if (urlObj.getPath().startsWith("http://")) {
        conn = (HttpURLConnection) this.urlObj.openConnection();
        Log.d(TAG, "prepare() : conn = " + conn);
        conn.setReadTimeout(8000);
        conn.setConnectTimeout(8000);
        this.imageSize = conn.getContentLength();
        try {
          inputStream = conn.getInputStream();
        } catch (IOException e) {
          Log.e(TAG, "conn.getInputStream", e);
        }
        if (inputStream == null) {
          try {
            conn.disconnect();
          } catch (Exception e1) {
          }
          throw new DecodeException("inputStream is null");
        }
      } else if (urlObj.getPath().startsWith("/")) {
        File file = new File(urlObj.getPath());
        Log.d(TAG, "file = " + file + ", urlObj.getPath() " + urlObj.getPath());
        inputStream = new FileInputStream(file);
        Log.d(TAG, "inputStream = " + inputStream);
      }
    } catch (IOException e) {
      Log.e(TAG, "openConnection", e);
    }
    BitmapFactory.Options bfOption = new BitmapFactory.Options();
    Log.d(TAG, "bfOption = " + bfOption);
    bfOption.inJustDecodeBounds = true;
    try {
      BitmapFactory.decodeStream(inputStream, null, bfOption);
    } catch (OutOfMemoryError e) {
      Log.e(TAG, "OutOfMemoryError decodeStream inJustDecodeBounds", e);
      try {
        inputStream.close();
        conn.disconnect();
      } catch (Exception e1) {
      }
      throw new DecodeOutOfMemoryException("out of memory");
    }
    Log.d(TAG, "bfOption.outWidth = " + bfOption.outWidth
        + ", bfOption.outHeight = " + bfOption.outHeight);
    if ((bfOption.outWidth > 0) && (bfOption.outHeight > 0)) {
      this.width = bfOption.outWidth;
      this.height = bfOption.outHeight;
    }
    if ((this.width <= 0) || (this.height <= 0)) {
      try {
        inputStream.close();
        conn.disconnect();
      } catch (IOException ioException) {
        Log.e(TAG, "IOException", ioException);
      }
      throw new DecodeException("width or height less than 0. width: "
          + this.width + " height: " + this.height);
    }
    try {
      inputStream.close();
      conn.disconnect();
    } catch (Exception e) {
    }

  }

  @Override
  public Bitmap decode() throws DecodeException {
    Bitmap bitmap = null;
    HttpURLConnection conn = null;
    InputStream inputStream = null;
    // try {
    // conn = (HttpURLConnection) this.urlObj.openConnection();
    // conn.setReadTimeout(8000);
    // conn.setConnectTimeout(8000);
    // } catch (IOException e) {
    // Log.e(TAG, "openConnection", e);
    // }
    // if (conn == null) {
    // throw new DecodeException("conn is null");
    // }
    try {
      if (urlObj.getPath().startsWith("http://")) {
        conn = (HttpURLConnection) this.urlObj.openConnection();
        Log.d(TAG, "prepare() : conn = " + conn);
        conn.setReadTimeout(8000);
        conn.setConnectTimeout(8000);
        this.imageSize = conn.getContentLength();
        try {
          inputStream = conn.getInputStream();
        } catch (IOException e) {
          Log.e(TAG, "conn.getInputStream", e);
        }
        try {
          inputStream = conn.getInputStream();
        } catch (IOException e) {
          Log.e(TAG, "conn.getInputStream", e);
        }
        if (inputStream == null) {
          try {
            conn.disconnect();
          } catch (Exception e1) {
          }
          throw new DecodeException("inputStream is null");
        }
      } else if (urlObj.getPath().startsWith("/")) {
        File file = new File(urlObj.getPath());
        Log.d(TAG, "file = " + file + ", urlObj.getPath() " + urlObj.getPath());
        inputStream = new FileInputStream(file);
        Log.d(TAG, "inputStream = " + inputStream);
      }
      if (inputStream == null) {
        try {
          conn.disconnect();
        } catch (Exception e1) {
        }
        throw new DecodeException("inputStream is null");
      }
    } catch (IOException exception) {
      Log.e(TAG, "IOException:", exception);
    }
    BitmapFactory.Options bfOption = new BitmapFactory.Options();
    bfOption.inJustDecodeBounds = false;
    bfOption.inPreferredConfig = Bitmap.Config.ARGB_8888;
    float bex = this.width / this.targetWidth;
    float bey = this.height / this.targetHeight;
    int be = (int) Math.max(Math.floor(bex), Math.floor(bey));
    if (be <= 0) {
      be = 1;
    }
    be = getSquareFloor(be);
    Log.d(TAG, "be=" + be);
    bfOption.inSampleSize = be;
    try {
      bitmap = BitmapFactory.decodeStream(inputStream, null, bfOption);
    } catch (OutOfMemoryError e) {
      Log.e(TAG, "OutOfMemoryError in BitmapFactory.decodeStream", e);
      try {
        inputStream.close();
        conn.disconnect();
      } catch (Exception e1) {
      }
      throw new DecodeOutOfMemoryException("decodeStream");
    }
    try {
      inputStream.close();
      conn.disconnect();
    } catch (Exception e1) {
    }
    return bitmap;
  }
}
