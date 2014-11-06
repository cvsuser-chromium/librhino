package com.hybroad.util;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.hybroad.media.R;

public class CustomWidget {
  private static Dialog sLoadIcon;

  public static void showLoadIcon(Context context) {
    if (sLoadIcon == null) {
      sLoadIcon = new Dialog(context, R.style.dialog_style_2);
      sLoadIcon.setContentView(R.layout.progress_dialog);
    }
    // if (!sLoadIcon.isShowing())
    sLoadIcon.show();
  }

  public static void hideLoadIcon() {
    if (null != sLoadIcon && sLoadIcon.isShowing()) {
      sLoadIcon.cancel();
    }
  }

  public static void releaseLoadIcon() {
    hideLoadIcon();
    sLoadIcon = null;
  }

  public static View getToastView(Context context, String content,
      float textSize) {
    LinearLayout rootLayout = new LinearLayout(context);
    rootLayout.setBackgroundResource(R.drawable.bg_menu_2);
    LinearLayout.LayoutParams rootLayoutParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.WRAP_CONTENT,
        LinearLayout.LayoutParams.WRAP_CONTENT);
    rootLayout.setLayoutParams(rootLayoutParams);

    TextView messageView = new TextView(context);
    messageView.setText(content);
    messageView.setTextColor(Color.rgb(255, 255, 255));
    messageView.setTextSize(TypedValue.COMPLEX_UNIT_SP, textSize);
    messageView.setPadding(15, 6, 15, 6); // setPadding(left, top, right,
                                          // bottom)
    LinearLayout.LayoutParams messageLayoutParams = new LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.WRAP_CONTENT,
        LinearLayout.LayoutParams.WRAP_CONTENT);

    rootLayout.addView(messageView, messageLayoutParams);

    return rootLayout;
  }

  public static void toastDeviceDown(Context context) {
    Toast toast = new Toast(context);
    toast.setGravity(Gravity.TOP, 0, 20);
    toast.setDuration(Toast.LENGTH_SHORT);
    toast.setView(CustomWidget.getToastView(context,
        context.getString(R.string.current_device_is_offline), 20));
    toast.show();
  }

  public static void toastNetworkDisconnect(Context context) {
    Toast toast = new Toast(context);
    toast.setGravity(Gravity.TOP, 0, 20);
    toast.setDuration(Toast.LENGTH_SHORT);
    toast.setView(CustomWidget.getToastView(context,
        context.getString(R.string.network_disconnect), 20));
    toast.show();
  }
}
