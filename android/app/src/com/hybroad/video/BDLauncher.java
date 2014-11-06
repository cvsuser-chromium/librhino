package com.hybroad.video;

import android.content.Intent;
import android.net.Uri;
import android.content.Context;

public class BDLauncher {
  public static void launchHiBDPlayer(Context context, String path) {
    Intent intent = new Intent();
    intent.setData(Uri.parse("bluray://"));
    intent.putExtra("path", path);
    intent.putExtra("isNetworkFile", false);
    context.startActivity(intent);
  }
}
