package com.hybroad.util;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

public class StatusBarOperator {
  public static void hideStatusBar(Context context) {
    showHideStatusBar(context, 1);
  }

  public static void showStatusBar(Context context) {
    showHideStatusBar(context, 0);
  }

  private static void showHideStatusBar(Context context, int flag) {
    if (null == context)
      return;
    Intent intent = new Intent();
    intent.setComponent(new ComponentName("com.android.systemui",
        "com.android.systemui.SystemUIService"));
    intent.putExtra("starttype", flag); // flag 0:show 1:hide
    context.startService(intent);
  }
}
