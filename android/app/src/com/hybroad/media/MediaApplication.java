package com.hybroad.media;

import android.app.Application;

public class MediaApplication extends Application {
  public static String APPLICATION_FILE_PATH;

  @Override
  public void onCreate() {
    super.onCreate();
    APPLICATION_FILE_PATH = getFilesDir().getAbsolutePath();
  }

}
