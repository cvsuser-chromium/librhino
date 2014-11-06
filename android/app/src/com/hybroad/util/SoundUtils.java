package com.hybroad.util;

import android.content.Context;
import android.media.AudioManager;

public class SoundUtils {
  private static AudioManager mAudioManager = null;
  private static int sMaxVolume = 0;

  public static boolean isMute(Context context) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    return 0 == mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
  }

  public static void setMute(Context context) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    if (isMute(context)) {
      mAudioManager.setStreamMute(AudioManager.STREAM_MUSIC, false);
    } else {
      mAudioManager.setStreamMute(AudioManager.STREAM_MUSIC, true);
    }
  }

  public static void setMute(Context context, boolean mute) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    mAudioManager.setStreamMute(AudioManager.STREAM_MUSIC, mute);
  }

  public static int getVolume(Context context) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    return mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
  }

  public static void setVolume(Context context, int targetVolume) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    int maxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    if (targetVolume > maxVolume) {
      targetVolume = maxVolume;
    } else if (targetVolume < 0) {
      targetVolume = 0;
    }
    mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, targetVolume, 1);
  }

  public static void setVolumePercent(Context context, int percent) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    if (0 == sMaxVolume) {
      sMaxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    }
    setVolume(context, sMaxVolume * percent / 100);
  }

  public static int getMaxVolume(Context context) {
    if (null == mAudioManager) {
      mAudioManager = (AudioManager) context
          .getSystemService(Context.AUDIO_SERVICE);
    }
    sMaxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    return sMaxVolume;
  }
}
