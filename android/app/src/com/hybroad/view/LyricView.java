package com.hybroad.view;

import java.util.ArrayList;

import com.hybroad.music.LrcUnit;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.widget.TextView;

public class LyricView extends TextView {
  private Paint mNotCurrentPaint;
  private Paint mCurrentPaint;
  private int mNotCurrentPaintColor = Color.WHITE;
  private int mCurrentPaintColor = Color.GREEN;
  private Typeface mTextTypeface = Typeface.DEFAULT;
  private float mWidth;
  private float mHeight;
  private int mBackgroundColor = 0xff000000;
  private float mLrcTextSize = 20;
  public float mTouchHistoryY;
  private int mTextGap = 50;
  public int mIndex = 0;
  private ArrayList<LrcUnit> mLrcSentenceList;

  public LyricView(Context context) {
    super(context);
    // TODO Auto-generated constructor stub
    init();
  }

  public LyricView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
    // TODO Auto-generated constructor stub
    init();
  }

  public LyricView(Context context, AttributeSet attrs) {
    super(context, attrs);
    // TODO Auto-generated constructor stub
    init();
  }

  public Paint getNotCurrentPaint() {
    return mNotCurrentPaint;
  }

  public void setNotCurrentPaint(Paint notCurrentPaint) {
    mNotCurrentPaint = notCurrentPaint;
  }

  public float getLrcTextSize() {
    return mLrcTextSize;
  }

  public void setLrcTextSize(float lrcTextSize) {
    this.mLrcTextSize = lrcTextSize;
    if (mCurrentPaint != null) {
      mCurrentPaint.setTextSize(lrcTextSize);
    }
    if (mNotCurrentPaint != null) {
      mNotCurrentPaint.setTextSize(lrcTextSize);
    }
  }

  public Paint getCurrentPaint() {
    return mCurrentPaint;
  }

  public void setCurrentPaint(Paint currentPaint) {
    mCurrentPaint = currentPaint;
  }

  public ArrayList<LrcUnit> getSentenceList() {
    return mLrcSentenceList;
  }

  public void setSentenceList(ArrayList<LrcUnit> sentenceList) {
    mLrcSentenceList = sentenceList;
  }

  public int getNotCurrentPaintColor() {
    return mNotCurrentPaintColor;
  }

  public void setNotCurrentPaintColor(int notCurrentPaintColor) {
    this.mNotCurrentPaintColor = notCurrentPaintColor;
    if (mNotCurrentPaint != null) {
      mNotCurrentPaint.setColor(notCurrentPaintColor);
    }
  }

  public int getCurrentPaintColor() {
    return mCurrentPaintColor;
  }

  public void setCurrentPaintColor(int currentPaintColor) {
    mCurrentPaintColor = currentPaintColor;
    if (mCurrentPaint != null) {
      mCurrentPaint.setColor(currentPaintColor);
    }
  }

  public Typeface getTexttypeface() {
    return mTextTypeface;
  }

  public void setTexttypeface(Typeface textTypeface) {
    mTextTypeface = textTypeface;
    if (mCurrentPaint != null) {
      mCurrentPaint.setTypeface(textTypeface);
    }
    if (mNotCurrentPaint != null) {
      mNotCurrentPaint.setTypeface(textTypeface);
    }
  }

  public int getLyricBackgroundColor() {
    return mBackgroundColor;
  }

  public void setLyricBackgroundColor(int backgroundColor) {
    this.mBackgroundColor = backgroundColor;
  }

  public int getTextGap() {
    return mTextGap;
  }

  public void setTextGap(int textGap) {
    mTextGap = textGap;
  }

  private void init() {
    mNotCurrentPaint = new Paint();
    mNotCurrentPaint.setAntiAlias(true);
    mNotCurrentPaint.setTextSize(mLrcTextSize);
    mNotCurrentPaint.setColor(mNotCurrentPaintColor);
    mNotCurrentPaint.setTypeface(mTextTypeface);
    mNotCurrentPaint.setTextAlign(Paint.Align.CENTER);
    mCurrentPaint = new Paint();
    mCurrentPaint.setAntiAlias(true);
    mCurrentPaint.setColor(mCurrentPaintColor);
    mCurrentPaint.setTextSize(mLrcTextSize);
    mCurrentPaint.setTypeface(mTextTypeface);
    mCurrentPaint.setTextAlign(Paint.Align.CENTER);
  }

  @Override
  protected void onDraw(Canvas canvas) {
    // TODO Auto-generated method stub
    super.onDraw(canvas);
    if (mLrcSentenceList != null && mIndex >= 0
        && mIndex < mLrcSentenceList.size()) {
      canvas.drawColor(mBackgroundColor);
      mNotCurrentPaint.setColor(mNotCurrentPaintColor);
      mCurrentPaint.setColor(mCurrentPaintColor);
      try {
        canvas.drawText(mLrcSentenceList.get(mIndex).getLrcBody(), mWidth / 2,
            mHeight / 2, mCurrentPaint);
        float tempY = mHeight / 2;
        for (int i = mIndex - 1; i >= 0; i--) {
          tempY = tempY - mTextGap;
          if (tempY < 0) {
            break;
          }
          canvas.drawText(mLrcSentenceList.get(i).getLrcBody(), mWidth / 2,
              tempY, mNotCurrentPaint);
        }
        tempY = mHeight / 2;
        for (int i = mIndex + 1; i < mLrcSentenceList.size(); i++) {
          tempY = tempY + mTextGap;
          if (tempY > mHeight) {
            break;
          }
          canvas.drawText(mLrcSentenceList.get(i).getLrcBody(), mWidth / 2,
              tempY, mNotCurrentPaint);
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    }
  }

  @Override
  protected void onSizeChanged(int w, int h, int oldw, int oldh) {
    // TODO Auto-generated method stub
    super.onSizeChanged(w, h, oldw, oldh);
    mWidth = w; // remember the center of the screen
    mHeight = h;
  }

  public int updateIndex(int index) {
    if (index < 0 || mLrcSentenceList == null
        || index >= mLrcSentenceList.size()) {
      mIndex = -1;
      return -1;
    }
    mIndex = index;
    return mLrcSentenceList.get(mIndex).getLineTime();
  }

}
