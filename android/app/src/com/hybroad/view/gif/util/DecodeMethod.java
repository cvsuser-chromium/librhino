package com.hybroad.view.gif.util;

import android.graphics.Bitmap;

public abstract class DecodeMethod {
  protected int width;
  protected int height;
  protected int targetWidth;
  protected int targetHeight;
  protected int imageSize;

  public abstract void prepare() throws DecodeException;

  public abstract Bitmap decode() throws DecodeException;

  public int getImageWidth() {
    return this.width;
  }

  public int getImageHeight() {
    return this.height;
  }

  public int getImageSize() {
    return this.imageSize;
  }

  protected int getSquareFloor(int num) {
    if (num == 1) {
      return 1;
    }
    for (int i = 1; i < 10; ++i) {
      if (num < Math.pow(2.0D, i)) {
        return (int) Math.pow(2.0D, i - 1);
      }
    }
    return num;
  }
}