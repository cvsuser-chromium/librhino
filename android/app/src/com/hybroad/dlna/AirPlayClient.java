/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hybroad.dlna;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

import com.hybroad.util.PublicConstants;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.util.Log;

@JNINamespace("rhino")
public class AirPlayClient {
  private final static String TAG = "AirPlayClient";
  // add by liumeidong 20140121 begin
  private final static boolean ISDEBUG = true;
  // add by liumeidong 20140121 end

  // The native side of this object.
  private int mNativeAirPlayClient = 0;

  private EventHandler mEventHandler;
  private PowerManager.WakeLock mWakeLock = null;
  private boolean mScreenOnWhilePlaying;

  // add liumeidong 20131111 begin
  private AudioTrack mAudioTrack;
  // add liumeidong 20131111 end
  private static Context mContext = null;

  /**
   * Default constructor. Consider using one of the create() methods for
   * synchronously instantiating a MediaPlayer from a Uri or resource.
   * <p>
   * When done with the AirPlayClient, you should call {@link #release()}, to
   * free the resources. If not released, too many MediaPlayer instances may
   * result in an exception.
   * </p>
   */
  private AirPlayClient() {

    Looper looper;
    // add by liumeidong 20140121 begin
    mHandler = new MyHandler();
    // add by liumeidong 20140121 end
    if ((looper = Looper.myLooper()) != null) {
      mEventHandler = new EventHandler(this, looper);
    } else if ((looper = Looper.getMainLooper()) != null) {
      mEventHandler = new EventHandler(this, looper);
    } else {
      mEventHandler = null;
    }

    // for test 20131203
    mNativeAirPlayClient = nativeInit(this);
    if (ISDEBUG)
      logd(TAG, "create AirPlayClient!");

  }

  /**
   * Convenience method to create a MediaPlayer for a given resource id. On
   * success, {@link #prepare()} will already have been called and must not be
   * called again.
   * <p>
   * When done with the MediaPlayer, you should call {@link #release()}, to free
   * the resources. If not released, too many MediaPlayer instances will result
   * in an exception.
   * </p>
   * 
   * @param context
   *          the Context to use
   * @return a MediaPlayer object, or null if creation failed
   */
  public static AirPlayClient create(Context context) {
    mContext = context;
    try {

      AirPlayClient dmp = new AirPlayClient();
      return dmp;
    } catch (IllegalArgumentException ex) {
      Log.d(TAG, "create failed:", ex);
      // fall through
    } catch (SecurityException ex) {
      Log.d(TAG, "create failed:", ex);
      // fall through
    }
    return null;
  }

  @Override
  protected void finalize() {
    Log.d(TAG, "DMRClient to finalize...AirPlayClient.this: "
        + AirPlayClient.this);
    ((NsdManager) mContext.getSystemService(Context.NSD_SERVICE))
        .unregisterService(mRegistrationListner);
    nativeDestroy(mNativeAirPlayClient);
  }

  /*
   * Do not change these values without updating their counterparts in
   * rhino/dlna/android/libdlna/IDLNAService.h!
   */
  private static final int EVENT_DLNA_NOP = 0; // interface test message
  private static final int EVENT_DLNA_DMP_EVENT_START = 1;
  private static final int EVENT_DLNA_DMSLIST_CHANGED = 2;
  private static final int EVENT_DLNA_FILELIST_CHANGE = 3;
  private static final int EVENT_DLNA_CHANNEL_RESOURCE_CHANGED = 4;
  private static final int EVENT_DLNA_DMP_EVENT_END = 99;

  private static final int EVENT_DLNA_DMR_EVENT_START = 100;
  private static final int EVENT_DLNA_DMR_EVENT_END = 199;

  public static final int EVENT_DLNA_ERROR = 400;
  private static final int EVENT_DLNA_INFO = 500;

  // DLNA ERROR TYPE
  public static final int DLNA_ERROR_UNKNOWN = 1;
  // 1xx
  public static final int DLNA_ERROR_SERVER_DIED = 100;
  // 2xx
  public static final int DLNA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200;

  private static final String DEFAULT_JSON = "{\"returncode\": \"0\"}";

  @SuppressLint("HandlerLeak")
  private class EventHandler extends Handler {
    private AirPlayClient m_dmp;

    public EventHandler(AirPlayClient dmp, Looper looper) {
      super(looper);
      m_dmp = dmp;
    }

    @Override
    public void handleMessage(Message msg) {
      // String value;
      if (m_dmp.mNativeAirPlayClient == 0) {
        Log.w(TAG, "dmr device went away with unhandled events");
        return;
      }
      HandleEventSync(msg.what, msg.obj);
      /*
       * if ((msg.obj instanceof String)) { Log.w(TAG,
       * "msg.obj is String object. value=" + msg.obj.toString()); value =
       * (String)msg.obj; } else { value = msg.obj.toString(); } Log.d(TAG,
       * "message type " + msg.what + " msg.obj=" + msg.obj.toString());
       * mOnAirPlayClientEvent.onEvent(m_dmp, msg.what, value);
       */
    }
  }

  NsdManager.RegistrationListener mRegistrationListner = new NsdManager.RegistrationListener() {

    @Override
    public void onRegistrationFailed(NsdServiceInfo arg0, int arg1) {
      // TODO Auto-generated method stub

    }

    @Override
    public void onServiceRegistered(NsdServiceInfo arg0) {
      // TODO Auto-generated method stub

    }

    @Override
    public void onServiceUnregistered(NsdServiceInfo arg0) {
      // TODO Auto-generated method stub

    }

    @Override
    public void onUnregistrationFailed(NsdServiceInfo arg0, int arg1) {
      // TODO Auto-generated method stub

    }

  };

  @CalledByNative
  private static int registerNsdServices(Object dmr_device_ref, String obj) {
    NsdServiceInfo nsdInfo = new NsdServiceInfo();

    AirPlayClient mp = (AirPlayClient) dmr_device_ref;
    if (mp == null) {
      return -1;
    }
    ((NsdManager) mContext.getSystemService(Context.NSD_SERVICE))
        .registerService(nsdInfo, NsdManager.PROTOCOL_DNS_SD,
            mp.mRegistrationListner);
    
    return -1;
  }

  /**
   * Called from native code when an interesting event happens. This method just
   * uses the EventHandler system to post the event back to the main app thread.
   * We use a weak reference to the original MediaPlayer object so that the
   * native code is safe from the object disappearing from underneath it. (This
   * is the cookie passed to native_setup().)
   */
  @CalledByNative
  private static void postEventFromNative(Object dmr_device_ref, int what,
      String obj) {
    // AirPlayClient mp = (AirPlayClient) ((WeakReference<AirPlayClient>)
    // dmr_device_ref)
    // .get();
    AirPlayClient mp = (AirPlayClient) dmr_device_ref;
    if (mp == null) {
      return;
    }

    if (mp.mEventHandler != null) {
      Message m = mp.mEventHandler.obtainMessage(what, 0, 0, obj);
      mp.mEventHandler.sendMessage(m);
    }
  }

  /**
   * Called from native code when an interesting event happens. This method just
   * uses the EventHandler system to post the event back to the main app thread.
   * We use a weak reference to the original MediaPlayer object so that the
   * native code is safe from the object disappearing from underneath it. (This
   * is the cookie passed to native_setup().)
   */
  @CalledByNative
  private static String postEventFromNativeSync(Object client_ref, int what,
      String obj) {
    // AirPlayClient mp = (AirPlayClient) ((WeakReference<?>) client_ref)
    // .get();
    AirPlayClient mp = (AirPlayClient) client_ref;
    if (mp == null) {
      return DEFAULT_JSON;
    }
    return mp.HandleEventSync(what, obj);
  }

  private String HandleEventSync(int what, Object obj) {
    Log.d(TAG, "HandleEventSync-----------AirPlayClient.this: "
        + AirPlayClient.this);
    String value = null;
    if (obj != null) {
      if ((obj instanceof String)) {
        Log.w(TAG, "obj is String object. value=" + obj.toString());
        value = (String) obj;
      } else {
        value = obj.toString();
      }
    }
    obj = null;
    Log.d(TAG, "message type " + what + " msg.obj=" + value);
    if (null != mOnAirPlayClientEvent)
      return mOnAirPlayClientEvent.onEvent(this, what, value);
    return DEFAULT_JSON;
  }

  /**
   * Interface definition for a callback to be invoked when playback of a media
   * source has completed.
   */
  public interface OnAirPlayClientEvent {
    String onEvent(AirPlayClient mp, int what, String obj);
  }

  private OnAirPlayClientEvent mOnAirPlayClientEvent = null;

  public void setAirPlayClientEventListener(OnAirPlayClientEvent listener) {
    mOnAirPlayClientEvent = listener;
  }

  private static native int nativeInit(Object AirPlayClient_this);

  private native void nativeDestroy(int nativeAirPlayClient);

  // add 20131108 begin
  /*
   * send messages to AirPlayer sender. jsonStr format: {"sender":"hybroad",
   * "status":"status"} status can be OnStop, OnPlay, OnPause.
   */
  public int announceToClients(String jsonStr) {
    Log.d(TAG, "announceToClients: jsonStr = " + jsonStr);
    return nativeAnnounceToClients(mNativeAirPlayClient, jsonStr);
  }

  private native int nativeAnnounceToClients(int nativeAirPlayClient,
      String jsonStr);

  @CalledByNative
  public String audioInit(int bits, int channels, int samplerateHZ) {

    String methodName = "audioInit";
    if (mAudioTrack != null) {
      mAudioTrack.stop();
      mAudioTrack.release();
      mAudioTrack = null;
    }
    if (ISDEBUG)
      logd(methodName, "audioTrack initialized..");
    int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    if (8 == bits) {
      audioFormat = AudioFormat.ENCODING_PCM_8BIT;
    }
    // add by liumeidong 20140121 begin
    AudioManager am = (AudioManager) mContext
        .getSystemService(Context.AUDIO_SERVICE);
    int result = am.requestAudioFocus(mAudioFocusChangeListener,
    // Use the music stream.
        AudioManager.STREAM_MUSIC,
        // Request permanent focus.
        AudioManager.AUDIOFOCUS_GAIN);
    if (ISDEBUG)
      logd(methodName, "airplayClient result = " + result);
    if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
      mState.audioFocusGranted = true;
    } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
      mState.audioFocusGranted = false;
    }

    mAudioFocusChangeListener = new AudioManager.OnAudioFocusChangeListener() {
      @Override
      public void onAudioFocusChange(int focusChange) {
        String methodName = "mAudioFocusChangeListener.onAudioFocusChange";
        switch (focusChange) {
        case AudioManager.AUDIOFOCUS_GAIN:
          mState.audioFocusGranted = true;
          Log.d(TAG, "grant the audio Focus!");
          break;
        case AudioManager.AUDIOFOCUS_LOSS:
          mState.audioFocusGranted = false;
          audioDestroy("-----loss the audo focus-----");
          if (ISDEBUG)
            logd(methodName, "loss the audio Focus!");
          break;
        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
          if (ISDEBUG)
            logd(methodName, "loss transient audio Focus!");
          mState.audioFocusGranted = false;
          break;
        }
      }
    };
    // add by liumeidong 20140121 end

    int minBufferSize = AudioTrack.getMinBufferSize(samplerateHZ,
        AudioFormat.CHANNEL_OUT_STEREO, audioFormat);
    mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, samplerateHZ,
        AudioFormat.CHANNEL_OUT_STEREO, audioFormat, minBufferSize,
        AudioTrack.MODE_STREAM);
    if (ISDEBUG)
      logd(methodName, "State = " + mAudioTrack.getState());
    if (mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED
        && mState.audioFocusGranted) {

      Intent intent = new Intent(PublicConstants.DMRIntent.ACTION_DMR_EVENT);
      intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
          PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP);
      mContext.sendBroadcast(intent);
      mHandler.sendEmptyMessageDelayed(MyHandler.DELAY_AUDIO_PLAY,
          MUSIC_PLAY_DELAY);
    }
    return "";
  }

  @CalledByNative
  public void audioSetVolumeByNative(String session, int volume) {
    // if (mAudioTrack != null) {
    // logd("audio_set_volume");
    // float leftVolume =(float) volume;
    // float rightVolume = (float) volume;
    // // mAudioTrack.setStereoVolume(leftVolume, rightVolume);
    // }
    if (mAudioTrack != null) {
      float leftVolume = ((AudioTrack.getMaxVolume() - AudioTrack
          .getMinVolume()) * volume) / 100;
      if (ISDEBUG)
        logd("audioSetVolumeByNative", "lefVolume = " + leftVolume);
      mAudioTrack.setStereoVolume(leftVolume, leftVolume);
    }
  }

  @CalledByNative
  public void audioSetMetadata(String session, String key, String value) {

    if (ISDEBUG)
      logd("audioSetMetadata", "key = " + key + ", value =" + value);
    if (key.equalsIgnoreCase("asal")) {
      // album

    } else if (key.equalsIgnoreCase("minm")) {
      // title

    } else if (key.equalsIgnoreCase("asar")) {
      // artist
    }

  }

  @CalledByNative
  public void audioSetCoverart(String session, byte[] buffer, int buflen) {
    if (ISDEBUG)
      logd("audio_set_coverart", session);
  }

  @CalledByNative
  public void audioDataProcess(final String session, final byte[] buffer,
      final int buflen) {
    if (buffer == null || buflen == 0) {
      return;
    }
    try {
      if (mAudioTrack != null) {
        mAudioTrack.write(buffer, 0, buflen);
      }
    } catch (Exception ex) {
      loge("audioDataProcess", ex);
    }
  }

  @CalledByNative
  public void audioDestroy(String session) {
    if (null != mAudioTrack) {
      if (ISDEBUG)
        logd("audio_destroy()", "session");
      mAudioTrack.stop();
      mAudioTrack.release();
      mAudioTrack = null;
    }
  }

  @CalledByNative
  public void audioFlush(String session) {

    if (mAudioTrack != null) {
      if (ISDEBUG)
        logd("audioFlush", session);
      mAudioTrack.flush();
    }
  }

  private void logd(String methodName, final String log) {
    Log.d(TAG, methodName + ", " + log);
  }

  private static void loge(String methodName, Exception exception) {
    Log.e(TAG, methodName, exception);
  }

  private AudioManager.OnAudioFocusChangeListener mAudioFocusChangeListener;

  class MusicPlayerStates {
    boolean audioFocusGranted = false;
    boolean userInitiatedState = false;
    boolean released = false;
  }

  private MusicPlayerStates mState = new MusicPlayerStates();

  private static final int MUSIC_PLAY_DELAY = 1000;
  private static MyHandler mHandler;

  @SuppressLint("HandlerLeak")
  private class MyHandler extends Handler {
    private static final int DELAY_AUDIO_PLAY = 1;

    @Override
    public void handleMessage(Message msg) {

      switch (msg.what) {
      case DELAY_AUDIO_PLAY:
        if (ISDEBUG)
          logd("handleMessage", "mAudioTrack = " + mAudioTrack);
        if (mAudioTrack != null
            && mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED) {
          logd("handleMessage", "mAudioTrack is playing");
          mAudioTrack.play();
        }
        break;
      default:
        break;
      }
      super.handleMessage(msg);
    }

  }
}
