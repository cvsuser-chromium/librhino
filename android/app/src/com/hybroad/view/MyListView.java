package com.hybroad.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.widget.ListView;

public class MyListView extends ListView {

  public MyListView(Context context) {
    super(context);
    // TODO Auto-generated constructor stub
  }

  public MyListView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
    // TODO Auto-generated constructor stub
  }

  public MyListView(Context context, AttributeSet attrs) {
    super(context, attrs);
    // TODO Auto-generated constructor stub
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    // TODO Auto-generated method stub

    if (keyCode == KeyEvent.KEYCODE_DPAD_CENTER
        || keyCode == KeyEvent.KEYCODE_DPAD_LEFT
        || keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
      return false;
    }

    return super.onKeyDown(keyCode, event);

  }

}
