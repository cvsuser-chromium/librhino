package com.hybroad.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.widget.SeekBar;

public class MySeekBar extends SeekBar {

  public MySeekBar(Context context) {
    super(context);
  }

  public MySeekBar(Context context, AttributeSet attrs) {
    super(context, attrs);
  }

  public MySeekBar(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    // TODO Auto-generated method stub
    return false;
  }

}
