package com.hybroad.image;

import android.graphics.Bitmap;

public class BitmapPackage {
    private Bitmap mBitmap = null;
    private int originalWidth = 0;
    private int originalHeight = 0;
    private int sampleValue = 1;
    private int mDegree = 0;

    public void setBitmap(Bitmap bitmap) {
        this.mBitmap = bitmap;
    }

    public void setOriginalWidth(int width) {
        this.originalWidth = width;
    }

    public void setOriginalHeight(int height) {
        this.originalHeight = height;
    }

    public void setSampleValue(int sample) {
        this.sampleValue = sample;
    }

    public void setDegreeDelta(int degree) {
        this.mDegree += degree;
        if (this.mDegree == 360 || this.mDegree == -360) {
            this.mDegree = 0;
        }
    }

    public Bitmap getBitmap() {
        return this.mBitmap;
    }

    public int getOriginalWidth() {
        return this.originalWidth;
    }

    public int getOriginalHeight() {
        return this.originalHeight;
    }

    public int getSampleValue() {
        return this.sampleValue;
    }

    public int getDegree() {
        return this.mDegree;
    }

    public boolean bitmapIsNull() {
        return mBitmap == null;
    }


}
