package com.hybroad.util;

import android.content.Context;
import android.content.SharedPreferences;

public class SharedPreferencesHelper {
  public static final String XML_MUSIC_SETTINGS = "music_player_settings";
  public static final String KEY_MUSIC_PLAY_MODE = "music_play_mode";

  public static final String XML_VIDEO_SETTINGS = "video_player_settings";
  public static final String KEY_VIDEO_PLAY_MODE = "video_play_mode";

  public static final String XML_VIDEO_EXPLORE_SETTINGS = "video_explore_settings";
  public static final String KEY_VIDEO_EXPLORE_SORT_RULE = "video_explore_sort_rule";

  public static final String XML_MUSIC_EXPLORE_SETTINGS = "music_explore_settings";
  public static final String KEY_MUSIC_EXPLORE_SORT_RULE = "music_explore_sort_rule";

  public static final String XML_IMAGE_EXPLORE_SETTINGS = "image_explore_settings";
  public static final String KEY_IMAGE_EXPLORE_SORT_RULE = "image_explore_sort_rule";

  public static void setValue(Context context, String preferenceName,
      String key, int value) {
    if (context == null || preferenceName == null || key == null) {
      return;
    }
    SharedPreferences sharedPreferences = context.getSharedPreferences(
        preferenceName, Context.MODE_PRIVATE);
    if (sharedPreferences == null) {
      return;
    }
    SharedPreferences.Editor editor = sharedPreferences.edit();
    if (editor == null) {
      return;
    }
    editor.putInt(key, value);
    editor.commit();
  }

  public static void setValue(Context context, String preferenceName,
      String key, String value) {
    if (context == null || preferenceName == null || key == null) {
      return;
    }
    SharedPreferences sharedPreferences = context.getSharedPreferences(
        preferenceName, Context.MODE_PRIVATE);
    if (sharedPreferences == null) {
      return;
    }
    SharedPreferences.Editor editor = sharedPreferences.edit();
    if (editor == null) {
      return;
    }
    editor.putString(key, value);
    editor.commit();
  }

  public static int getValue(Context context, String preferenceName,
      String key, int defaultValue) {
    if (context == null || preferenceName == null || key == null) {
      return defaultValue;
    }
    SharedPreferences sharedPreferences = context.getSharedPreferences(
        preferenceName, Context.MODE_PRIVATE);
    if (sharedPreferences == null) {
      return defaultValue;
    }
    int value = defaultValue;
    try {
      value = sharedPreferences.getInt(key, defaultValue);
    } catch (Exception e) {
      e.printStackTrace();
    }
    return value;
  }

  public static String getValue(Context context, String preferenceName,
      String key, String defaultValue) {
    if (context == null || preferenceName == null || key == null) {
      return defaultValue;
    }
    SharedPreferences sharedPreferences = context.getSharedPreferences(
        preferenceName, Context.MODE_PRIVATE);
    if (sharedPreferences == null) {
      return defaultValue;
    }
    String value = defaultValue;
    try {
      value = sharedPreferences.getString(key, defaultValue);
    } catch (Exception e) {
      e.printStackTrace();
    }
    return value;
  }
}
