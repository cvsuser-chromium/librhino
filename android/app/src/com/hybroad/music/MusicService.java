package com.hybroad.music;

import java.io.IOException;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.IBinder;
import android.os.Parcel;
import android.os.PowerManager;
import android.text.TextUtils;
import android.util.Log;

import com.hybroad.util.PublicConstants;
import com.hybroad.video.VideoConstant;

public class MusicService extends Service implements
    MediaPlayer.OnPreparedListener, MediaPlayer.OnCompletionListener,
    MediaPlayer.OnErrorListener {
  private static final String TAG = "----MusicService----";

  // all possible internal states
  private static final int STATE_ERROR = -1;
  private static final int STATE_IDLE = 0;
  private static final int STATE_INITIALIZED = 1;
  private static final int STATE_PREPARING = 2;
  private static final int STATE_PREPARED = 3;
  private static final int STATE_PLAYING = 4;
  private static final int STATE_PAUSED = 5;
  private static final int STATE_STOPED = 6;
  private static final int STATE_PLAYBACK_COMPLETED = 7;
  private static final int STATE_RELEASED = 8;

  private int mCurrentState = STATE_IDLE;
  private int mTargetState = STATE_IDLE;

  private int intMusicDuration;
  private long lastSeekTime = 0;
  private static boolean runFlushTimeThread = false;
  private int mSeekWhenPrepared = 0;
  private boolean isPlayingBeforeScreenOff = false;
  private boolean isSeeking = false;

  private MediaPlayer mMediaPlayer = null;
  private String audioFilePath = null;
  private Thread flushTimeThread = null;
  private MusicServiceReceiver mServiceMediaReceiver;
  private MusicServiceReceiver mPlayCMDReceiver;
  private MusicServiceReceiver mServicePowerReceiver;

  @Override
  public void onCreate() {
    super.onCreate();
    Log.d(TAG, "--onCreate--");

    mPlayCMDReceiver = new MusicServiceReceiver();
    IntentFilter mPlayCMDFilter = new IntentFilter();
    mPlayCMDFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
    registerReceiver(mPlayCMDReceiver, mPlayCMDFilter);

    mServicePowerReceiver = new MusicServiceReceiver();
    IntentFilter mServicePowerFilter = new IntentFilter();
    mServicePowerFilter.addAction(Intent.ACTION_SCREEN_OFF);
    mServicePowerFilter.addAction(Intent.ACTION_SCREEN_ON);
    registerReceiver(mServicePowerReceiver, mServicePowerFilter);

    mServiceMediaReceiver = new MusicServiceReceiver();
    IntentFilter mServiceMediaFilter = new IntentFilter();
    mServiceMediaFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
    mServiceMediaFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
    mServiceMediaFilter.addAction(Intent.ACTION_MEDIA_EJECT);
    mServiceMediaFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
    mServiceMediaFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
    mServiceMediaFilter.addDataScheme("file"); // important!!!
    registerReceiver(mServiceMediaReceiver, mServiceMediaFilter);

    mCurrentState = STATE_IDLE;
    mTargetState = STATE_IDLE;
    startFlushTimeThread();
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    Log.d(TAG, "--onStartCommand--");

    if (intent != null) {
      audioFilePath = intent
          .getStringExtra(PublicConstants.MusicCommon.INTENT_KEY_MUSIC_FILE_PATH);
      Log.d(TAG, "--audioFilePath--" + audioFilePath);
      mSeekWhenPrepared = intent.getIntExtra(
          PublicConstants.MusicCommon.INTENT_KEY_SEEK_VALUE, 0);
    }

    if (!TextUtils.isEmpty(audioFilePath)) {
      openMusic(audioFilePath);
    }

    return super.onStartCommand(intent, flags, startId);
  }

  @Override
  public void onDestroy() {
    Log.d(TAG, "--onDestroy--");
    super.onDestroy();
    release(true);

    stopFlushTimeThread();

    unregisterReceiver(mPlayCMDReceiver);
    unregisterReceiver(mServicePowerReceiver);
    unregisterReceiver(mServiceMediaReceiver);
  }

  @Override
  public IBinder onBind(Intent arg0) {
    Log.d(TAG, "--onBind--");
    return null;
  }

  @Override
  public void onPrepared(MediaPlayer mp) {
    mCurrentState = STATE_PREPARED;
    sendDuration();

    Intent intent = new Intent();
    intent.setAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_PREPARED);
    sendBroadcast(intent);

    try {
      // mSeekWhenPrepared may be changed after seekTo() call
      int seekToPosition = mSeekWhenPrepared;
      if (seekToPosition != 0) {
        seekTo(seekToPosition);
      }
      if (mTargetState == STATE_PLAYING) {
        start();
      }
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
    }
  }

  @Override
  public void onCompletion(MediaPlayer mp) {
    Log.d(TAG, "--onCompletion--");
    mCurrentState = STATE_PLAYBACK_COMPLETED;
    mTargetState = STATE_PLAYBACK_COMPLETED;
    Intent intent = new Intent();
    intent.setAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_FINISH);
    sendBroadcast(intent);
  }

  @Override
  public boolean onError(MediaPlayer mp, int framework_err, int impl_err) {
    mCurrentState = STATE_ERROR;
    mTargetState = STATE_ERROR;
    switch (framework_err) {
    case MediaPlayer.MEDIA_ERROR_SERVER_DIED:
      if (mMediaPlayer != null) {
        mMediaPlayer.release();
      }
      mSeekWhenPrepared = 0;
      /*
       * Creating a new MediaPlayer and settings its wakemode does not require
       * the media service, so it's OK to do this now, while the service is
       * still being restarted
       */
      mMediaPlayer = new MediaPlayer();
      mMediaPlayer.setWakeMode(MusicService.this,
          PowerManager.PARTIAL_WAKE_LOCK);
      Log.e(TAG, "Error: MEDIA_ERROR_SERVER_DIED what=" + framework_err
          + ", extra=" + impl_err
          + "; release MediaPlayer and reset MediaPlayer");
      return true;
    default:
      if (mMediaPlayer != null) {
        mMediaPlayer.release();
      }
      mSeekWhenPrepared = 0;
      Log.e(TAG, "Error: Song format is not correct! default what="
          + framework_err + ",extra=" + impl_err
          + "; release MediaPlayer and reset MediaPlayer");
      Intent intent = new Intent();
      intent.setAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR);
      sendBroadcast(intent);
      break;
    }
    return false;
  }

  private void openMusic(String path) {
    if (TextUtils.isEmpty(path)) {
      return;
    }
    if (mMediaPlayer != null) {
      mMediaPlayer.release();
      mMediaPlayer = null;
    }
    try {
      mMediaPlayer = new MediaPlayer();
      mMediaPlayer.setOnPreparedListener(this);
      mMediaPlayer.setOnCompletionListener(this);
      mMediaPlayer.setOnErrorListener(this);
      if (path.startsWith("content://")) {
        mMediaPlayer.setDataSource(MusicService.this, Uri.parse(path));
      } else {
        mMediaPlayer.setDataSource(path);
      }
      /*
       * Must call this method before prepare() or prepareAsync() in order for
       * the target stream type to become effective thereafter.
       */
      mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
      /*
       * After setting the datasource and the display surface, you need to
       * either call prepare() or prepareAsync(). For streams, you should call
       * prepareAsync(), which returns immediately, rather than blocking until
       * enough data has been buffered.
       */
      mCurrentState = STATE_INITIALIZED;
      mMediaPlayer.prepareAsync();
      mCurrentState = STATE_PREPARING;
      start();
      if (flushTimeThread.isInterrupted()) {
        Log.d(TAG, "--flushTimeThread.isInterrupted--");
      }
      if (!flushTimeThread.isAlive()) {
        Log.d(TAG, "--flushTimeThread is not alive--");
        startFlushTimeThread();
      }
    } catch (IOException ex) {
      Log.w(TAG, "Unable to open content: " + path, ex);
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
      return;
    } catch (IllegalArgumentException ex) {
      Log.w(TAG, "Unable to open content: " + path, ex);
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
      return;
    }
  }

  public boolean isInPlaybackState() {
    return (mMediaPlayer != null && mCurrentState != STATE_IDLE
        && mCurrentState != STATE_INITIALIZED
        && mCurrentState != STATE_PREPARING && mCurrentState != STATE_STOPED
        && mCurrentState != STATE_RELEASED && mCurrentState != STATE_ERROR);
  }

  private void start() {
    try {
      if (isInPlaybackState()) {
        mMediaPlayer.start();
        mCurrentState = STATE_PLAYING;
      }
      mTargetState = STATE_PLAYING;
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
    }
  }

  private void pause() {
    try {
      if (isInPlaybackState()) {
        if (mMediaPlayer.isPlaying()) {
          mMediaPlayer.pause();
          mCurrentState = STATE_PAUSED;
        }
      }
      mTargetState = STATE_PAUSED;
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
    }
  }

  private void stop() {
    try {
      if (mMediaPlayer != null && mCurrentState != STATE_IDLE
          && mCurrentState != STATE_INITIALIZED
          && mCurrentState != STATE_PREPARING
          && mCurrentState != STATE_RELEASED && mCurrentState != STATE_ERROR) {
        mMediaPlayer.stop();
        mCurrentState = STATE_STOPED;
      }
      mTargetState = STATE_STOPED;
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
    }
  }

  public void release(boolean clearTargetState) {
    if (mMediaPlayer != null) {
      // mMediaPlayer.reset();
      // mCurrentState = STATE_IDLE;
      mMediaPlayer.release();
      mCurrentState = STATE_RELEASED;
      mMediaPlayer = null;
      if (clearTargetState) {
        mTargetState = STATE_IDLE;
      }
    }
  }

  public int getDuration() {
    try {
      if (isInPlaybackState())
        intMusicDuration = mMediaPlayer.getDuration();
      else
        intMusicDuration = 0;
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
      return 0;
    }
    return intMusicDuration;
  }

  private void sendDuration() {
    intMusicDuration = getDuration();
    Intent tempIntent = new Intent();
    tempIntent.setAction(PublicConstants.MusicCommon.INTENT_ACTION_DURATION);
    tempIntent.putExtra(PublicConstants.MusicCommon.INTENT_KEY_DURATION,
        intMusicDuration);
    sendBroadcast(tempIntent);
    Log.d(TAG, "--send: MusicDuration----" + intMusicDuration);
  }

  public int getCurrentPosition() {
    int currentPosition = 0;
    try {
      if (isInPlaybackState())
        currentPosition = mMediaPlayer.getCurrentPosition();
      else
        currentPosition = 0;
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
      return 0;
    }
    return currentPosition;
  }

  public void seekTo(int msec) {
    if (isInPlaybackState()) {
      try {
        mMediaPlayer.seekTo(msec);
        mSeekWhenPrepared = 0;
      } catch (Exception e) {
        e.printStackTrace();
        this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
        return;
      }
    } else {
      mSeekWhenPrepared = msec;
    }
  }

  public boolean isPlaying() {
    boolean isPlaying = false;
    try {
      isPlaying = isInPlaybackState() && mMediaPlayer.isPlaying();
    } catch (Exception e) {
      e.printStackTrace();
      this.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
      return false;
    }
    return isPlaying;
  }

  private void startFlushTimeThread() {
    runFlushTimeThread = true;
    flushTimeThread = new Thread(new ThreadRunnableCenter(
        MusicConstant.THREAD_ID_FLUSH_TIME));
    flushTimeThread.start();
    Log.d(TAG, "--start a flushTimeThread!--");
  }

  private void stopFlushTimeThread() {
    runFlushTimeThread = false;
    flushTimeThread = null;
  }

  class ThreadRunnableCenter implements Runnable {
    private int id = -1;
    private Intent intent;

    public ThreadRunnableCenter(int id) {
      ThreadRunnableCenter.this.id = id;
      if (id == MusicConstant.THREAD_ID_FLUSH_TIME) {
        intent = new Intent(
            PublicConstants.MusicCommon.INTENT_ACTION_FLUSH_TIME);
      }
    }

    @Override
    public void run() {
      try {
        switch (id) {
        case MusicConstant.THREAD_ID_FLUSH_TIME:
          while (runFlushTimeThread) {
            if (System.currentTimeMillis() - lastSeekTime >= 500) {
              if ((!isSeeking) && isInPlaybackState()) {
                intent.putExtra("intCurrentPosition", getCurrentPosition());
                sendBroadcast(intent);
                if (intMusicDuration <= 0) {
                  sendDuration();
                }
              }
            }
            Thread.sleep(998);
          }
          break;

        default:
          break;
        }
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }

  class MusicServiceReceiver extends BroadcastReceiver {
    private Intent seekIntent = new Intent(
        PublicConstants.MusicCommon.INTENT_ACTION_FLUSH_TIME);

    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      if (action.equals(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD)) {
        switch (intent.getExtras().getInt("playerCommand")) {
        case MusicConstant.CMD_PLAY:
          start();
          Log.d(TAG, "--playerCommand--CMD_PLAY");
          break;
        case MusicConstant.CMD_PAUSE:
          pause();
          Log.d(TAG, "--playerCommand--CMD_PAUSE");
          break;
        case MusicConstant.CMD_STOP:
          stop();
          Log.d(TAG, "--playerCommand--CMD_STOP");
          break;
        case MusicConstant.CMD_SEEK:
          isSeeking = true;
          lastSeekTime = System.currentTimeMillis();
          seekTo(intent.getExtras().getInt("seekValue"));
          seekIntent.putExtra("intCurrentPosition", getCurrentPosition());
          sendBroadcast(seekIntent);
          isSeeking = false;
          Log.d(TAG, "--playerCommand--CMD_SEEK");
          break;
        case MusicConstant.CMD_SET_CHANNEL:
          Log.d(TAG, "--playerCommand--CMD_SET_CHANNEL");
          setAudioChannel(intent.getExtras().getInt("channelID"));
          break;
        case MusicConstant.CMD_SET_LR_VOLUME:
          Log.d(TAG, "--playerCommand--CMD_SET_LR_VOLUME");
          setLRVolume(intent.getExtras().getFloat("left"), intent.getExtras()
              .getFloat("right"));
          break;

        default:
          break;
        }
      } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
        if (isPlaying()) {
          isPlayingBeforeScreenOff = true;
          pause();
        } else {
          isPlayingBeforeScreenOff = false;
        }
      } else if (action.equals(Intent.ACTION_SCREEN_ON)) {
        if (isPlayingBeforeScreenOff) {
          start();
        }
      } else if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
        /*
         * User has expressed the desire to remove the external storage media.
         * Applications should close all files they have open within the mount
         * point when they receive this intent.
         */
        stop();
      } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {

      } else if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {

      }

    } // end onReceive

  } // end MusicUIReceiver

  private void setAudioChannel(int channelID) {
    Parcel requestParcel = Parcel.obtain();
    requestParcel.writeInterfaceToken("android.media.IMediaPlayer");
    requestParcel.writeInt(VideoConstant.CMD_SET_CHANNEL);
    requestParcel.writeInt(channelID);
    Parcel replayParcel = Parcel.obtain();
    if (isInPlaybackState()) {
      Log.d(TAG, "--MediaPlayer.invoke--CMD_SET_CHANNEL: " + channelID);
//      mMediaPlayer.invoke(requestParcel, replayParcel);
    }
  }

  public void setLRVolume(float arg0, float arg1) {
    if (isInPlaybackState()) {
      mMediaPlayer.setVolume(arg0, arg1);
    }
  }
}
