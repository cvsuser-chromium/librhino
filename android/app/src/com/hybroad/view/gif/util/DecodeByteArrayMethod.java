package com.hybroad.view.gif.util;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

public class DecodeByteArrayMethod extends DecodeMethod {
  private static final String TAG = "DecodeByteArrayMethod";
  private URL urlObj;
  private byte[] buf;

  public DecodeByteArrayMethod(URL urlObj, int targetWidth, int targetHeight) {
    this.urlObj = urlObj;
    this.targetWidth = targetWidth;
    this.targetHeight = targetHeight;
  }

  public void prepare() throws DecodeException {
    HttpURLConnection conn = null;
    try {
      conn = (HttpURLConnection) this.urlObj.openConnection();
      conn.setReadTimeout(8000);
      conn.setConnectTimeout(8000);
      this.imageSize = conn.getContentLength();
    } catch (IOException e) {
      Log.e(TAG, "openConnection", e);
    }
    if (conn == null) {
      throw new DecodeException("conn is null");
    }
    if (this.imageSize <= 0) {
      throw new DecodeException("image size less than 0: " + this.imageSize);
    }
    InputStream inputStream = null;
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

    BufferedInputStream bis = null;
    try {
      bis = new BufferedInputStream(inputStream);
      int index = 0;
      int offset = 0;
      byte[] tmpBuf = new byte[16384];
      this.buf = new byte[this.imageSize];
      while ((index = bis.read(tmpBuf)) != -1) {
        System.arraycopy(tmpBuf, 0, this.buf, offset, index);
        offset += index;
      }
      tmpBuf = null;
      if (offset != this.imageSize)
        throw new DecodeException("offset is not equal with imageSize");
    } catch (Exception e) {
      Log.e(TAG, "read buf: ", e);
      try {
        bis.close();
        inputStream.close();
        conn.disconnect();
      } catch (Exception e1) {
      }
      throw new DecodeException("read buf");
    } catch (OutOfMemoryError e) {
      Log.e(TAG, "new byte[" + this.imageSize + "]", e);
      try {
        bis.close();
        inputStream.close();
        conn.disconnect();
      } catch (Exception e1) {
      }
      throw new DecodeOutOfMemoryException("new byte[" + this.imageSize + "]");
    }
    try {
      bis.close();
      inputStream.close();
      conn.disconnect();
    } catch (Exception e) {
    }
    if (this.buf == null) {
      Log.e(TAG, "buf is null");
      throw new DecodeException("buf is null");
    }
    BitmapFactory.Options bfOption = new BitmapFactory.Options();
    bfOption.inJustDecodeBounds = true;
    try {
      BitmapFactory.decodeByteArray(this.buf, 0, this.buf.length, bfOption);
    } catch (OutOfMemoryError e) {
      Log.e(TAG, "OutOfMemoryError decodeStream inJustDecodeBounds", e);
      throw new DecodeOutOfMemoryException("out of memory");
    }
    if ((bfOption.outWidth > 0) && (bfOption.outHeight > 0)) {
      this.width = bfOption.outWidth;
      this.height = bfOption.outHeight;
    }

    if ((this.width > 0) && (this.height > 0))
      return;
    try {
      inputStream.close();
      conn.disconnect();
    } catch (Exception e1) {
    }
    throw new DecodeException("width or height less than 0. width: "
        + this.width + " height: " + this.height);
  }

  public Bitmap decode() throws DecodeException {
    Bitmap bitmap = null;
    if (this.buf == null) {
      Log.e(TAG, "buf is null");
      throw new DecodeException("buf is null");
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
      bitmap = BitmapFactory.decodeByteArray(this.buf, 0, this.buf.length,
          bfOption);
    } catch (OutOfMemoryError e) {
      Log.e(TAG, "decodeByteArray", e);
      this.buf = null;
      throw new DecodeOutOfMemoryException("decodeByteArray");
    }
    this.buf = null;
    return bitmap;
  }
}