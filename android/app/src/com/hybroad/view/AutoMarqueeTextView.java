package com.hybroad.view;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.TextView;

public class AutoMarqueeTextView extends TextView {

  public AutoMarqueeTextView(Context context) {
    super(context);
    // TODO Auto-generated constructor stub
  }

  public AutoMarqueeTextView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
    // TODO Auto-generated constructor stub
  }

  public AutoMarqueeTextView(Context context, AttributeSet attrs) {
    super(context, attrs);
    // TODO Auto-generated constructor stub
  }

  @Override
  public boolean isFocused() {
    // TODO Auto-generated method stub
    return true; // always true!!!
  }

  @Override
  protected void onFocusChanged(boolean focused, int direction,
      Rect previouslyFocusedRect) {
    // TODO Auto-generated method stub
    // do NOTHING!!!
  }

}
