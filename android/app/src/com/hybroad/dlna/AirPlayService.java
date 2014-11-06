package com.hybroad.dlna;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;

import com.hybroad.dlna.aidl.IAirPlayService;
import com.hybroad.dlna.aidl.IVideoPlayerCallback;
import com.hybroad.json.DMRPlayJSON;
import com.hybroad.json.DMRSeekJSON;
import com.hybroad.json.DMRSetMuteJSON;
import com.hybroad.json.DMRSetVolumeJSON;
import com.hybroad.json.JSONUtils;
import com.hybroad.json.TrackMetaDataJSON;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.SoundUtils;

public class AirPlayService extends Service {
  private static final String TAG = "AirPlayService";
  private static final boolean ISDEBUG = true;

  private static final String PLAY_URL = "PlayUrl";
  private static final String MEDIA_TYPE = "mediaType";
  private static final String CHANNEL = "Channel";
  private static final String VOLUME = "Volume";
  private static final String SLIDESHOW = "slidedShow";
  private static AirPlayClient mAirPlayClient = null;
  private AirPlayServiceBinder mAirPlayServiceBinder = null;
  private IVideoPlayerCallback mCallback = null;

  @Override
  public void onCreate() {
    Log.d(TAG, "onCreate");
    super.onCreate();
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    if (ISDEBUG) logd("onStartCommand", " ");
    if (null == mAirPlayClient) {
      LibraryLoader.ensureInitialized();
      mAirPlayClient = AirPlayClient.create(AirPlayService.this);
      setAirPlayClientListner(AirPlayService.this);
    }

    return START_STICKY;
  }

  @Override
  public void onDestroy() {
    Log.d(TAG, "onDestroy");
    Intent intent = new Intent();
    intent.setClass(this, DMRService.class);
    this.startService(intent);
  }

  private void setAirPlayClientListner(final Context context) {
    final String methodName = "setAirPlayClientListner.onEvent";
    if (null == mAirPlayClient) {
      logd(methodName, "mAirPlayClient is null");
      return;
    }

    mAirPlayClient
        .setAirPlayClientEventListener(new AirPlayClient.OnAirPlayClientEvent() {
          @Override
          public String onEvent(AirPlayClient mp, int what, String jsonString) {
            if (ISDEBUG) logd(methodName, "what = " + what + ", jsonString = " + jsonString);

            Intent intent = new Intent(
                PublicConstants.DMRIntent.ACTION_DMR_EVENT);
            String returnJson = "{ \"returncode\": \"-911\" }";

            if (TextUtils.isEmpty(jsonString)) {
              logd(methodName, "jsonString is empty");
              return null;
            }
            switch (what) {
            case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETMUTE:
              if (ISDEBUG) logd(methodName, "to set mute ");
              getMuteParamFromJson(jsonString, context);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME:
              if (ISDEBUG) logd(methodName, "to set volume ");
              getVolumeParamFromJson(jsonString, intent, context);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETMUTE:
              int mute = SoundUtils.isMute(context) ? 1 : 0;
              returnJson = "{ \"Mute\": \"" + mute + "\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETVOLUME:
              int volume = SoundUtils.getVolume(context);
              int max = SoundUtils.getMaxVolume(context);
              int percent = (int) ((float) volume / max * 100);
              returnJson = "{ \"Volume\": \"" + percent + "\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY:
              getPlayParamsFromJsonStr(jsonString, intent, context);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE:
              if (ISDEBUG) logd(methodName, "to set pause ");
              intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                  PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE);
              context.sendBroadcast(intent);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME: // no
                                                                 // use
              intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                  PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME);
              context.sendBroadcast(intent);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK:
              if (ISDEBUG) logd(methodName, "to set seek ");
              getSeekParamFromJsonStr(jsonString, intent, context);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP:
              if (ISDEBUG) logd(methodName, "to set stop ");
              intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                  PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP);
              context.sendBroadcast(intent);
              // confirmReallyStop(context, intent);
              returnJson = "{ \"returncode\": \"0\" }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETPOSITIONINFO:
              String url = "";

              // Don't send TrackMetaData to server, or server
              // will not send
              // TrackMetaData to DMC.
              try {

                if (mCallback != null) {
                  returnJson = mCallback.getCurrentPositionInfo();
                  Log.d(TAG, "returnJson" + returnJson);
                } 
              } catch (RemoteException e) {
                e.printStackTrace();
              }
              Log.i(TAG, "GETPOSITIONINFO: " + returnJson);
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETTRANSPORTINFO:
              returnJson = "{ " + "\"CurrentTransportState\": \""
                  + DMRSharingInformation.sTransportState + "\", "
                  + "\"CurrentTransportStatus\": \""
                  + DMRSharingInformation.sTransportStatus + "\", "
                  + "\"CurrentSpeed\": \"1\"" + " }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETMEDIAINFO:
              url = "";
              if (!TextUtils.isEmpty(DMRSharingInformation.sCurrentPlayUrl))
                url = DMRSharingInformation.sCurrentPlayUrl;

              returnJson = "{ " + "\"CurrentURI\": \"" + url + "\", "
                  + "\"status\": \"" + "\"OnPlay\"" + "\", "
                  + "\"NrTracks\": \"0\", "
                  + "\"WriteStatus\": \"NOT_IMPLEMENTED\", "
                  + "\"CurrentURIMetaData\": \"\", " + "\"NextURI\": \"\", "
                  + "\"NextURIMetaData\": \"\", " + "\"PlayMedium\": \"\", "
                  + "\"RecordMedium\": \"\"" + " }";
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM:
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETPRODUCTINFO:
              break;

            case PublicConstants.DMREvent.DLNA_EVENT_DMR_ORDER:
              break;

            default:
              break;
            }

            if (ISDEBUG) logd(methodName, "returnJson " + returnJson);
            return returnJson;
          }
        });
  }

  private boolean getSeekParamFromJsonStr(String jsonStr, Intent intent,
      Context context) {

    String methodName = "getSeekParamFromJsonStr";
    DMRSeekJSON seekJSON = new DMRSeekJSON();
    seekJSON = (DMRSeekJSON) JSONUtils.loadObjectFromJSON(jsonStr, seekJSON);
    if (null == seekJSON) {
      logd(methodName, "seekJSON is null");
      return false;
    }
    if (TextUtils.isEmpty(seekJSON.getSeek_target())) {
      logd(methodName, "seekJSON.getSeek_target is empty");
      return false;
    }
    // add
    // int seek = (int) Converter
    // .convertHoursMinutesSecondsToMsec(seekJSON
    // .getSeek_target());
    double origSeek = Double.parseDouble(seekJSON.getSeek_target());
    int seek = (int) origSeek;
    if (ISDEBUG) logd(methodName, "seek: " + seek);
    DMRSharingInformation.sInitSeekPosition = seek; // for
                                                    // first
                                                    // seek
    intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
        PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK);
    intent.putExtra(PublicConstants.DMRIntent.KEY_SEEK_VALUE, seek);
    context.sendBroadcast(intent);
    return true;
  }

  // get three media form params for play
  private boolean getPlayParamsFromJsonStr(String jsonString, Intent intent,
      Context context) {

    String methodName = "getPlayParamsFromJsonStr";
    String playType = null;
    String originalTitle = null;
    String tempUrl = null;
    boolean isSlideShow = false;
    JSONObject jsonObject;

    try {

      // add for DLNAPLAY begin
      DMRPlayJSON playJson = new DMRPlayJSON();
      playJson = (DMRPlayJSON) JSONUtils.loadObjectFromJSON(jsonString,
          playJson);
      if (playJson == null) {
        logd(methodName, "playJson is null");
        return false;
      }
      // add for DLNAPLAY end

      jsonObject = (JSONObject) new JSONTokener(jsonString).nextValue();
      if (jsonObject == null) {
        logd(methodName, "jsonObject is null");
        return false;
      }

      if (jsonObject.has(PLAY_URL)) {
        tempUrl = jsonObject.getString(PLAY_URL);
      }

      if (TextUtils.isEmpty(tempUrl)) {
        return false;
      }

      // if (tempUrl.equals(DMRSharingInformation.sCurrentPlayUrl)) {
      // Log.d(TAG, "getPlayParamsFromJsonStr: play from pause: URL = " +
      // tempUrl);
      // intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
      // PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY);
      // context.sendBroadcast(intent);
      // } else {
      DMRSharingInformation.sCurrentPlayUrl = tempUrl;
      // add for DLNAPLAY begin
      TrackMetaDataJSON TrackMetadata = playJson.getTrackMetadata();
      if (null != TrackMetadata) {
        originalTitle = TrackMetadata.getDc_title();
        if (ISDEBUG) logd(methodName, "originalTitle: " + originalTitle);
      }
      // add for DLNAPLAY end
      if (jsonObject.has(MEDIA_TYPE)) {
        playType = jsonObject.getString(MEDIA_TYPE);
        if (ISDEBUG) logd(methodName, "playType: " + playType);
      }

      if (!TextUtils.isEmpty(playType)
          && PublicConstants.DLNACommon.MEDIA_TYPE_VIDEO.equals(playType)) {
        startVideoPlayer(context, jsonString);
      } else if (!TextUtils.isEmpty(playType)
          && PublicConstants.DLNACommon.MEDIA_TYPE_PICTURE.equals(playType)) {
        if (jsonObject.has(SLIDESHOW)) {
          isSlideShow = jsonObject.getBoolean(SLIDESHOW);
          if (ISDEBUG) logd(methodName, "isSlideShow = " + isSlideShow);
        }

        startImagePlayer(context, DMRSharingInformation.sCurrentPlayUrl,
            originalTitle, isSlideShow);
      } else if (PublicConstants.DLNACommon.MEDIA_TYPE_AUDIO.equals(playType)) {
        startMusicPlayer(context, DMRSharingInformation.sCurrentPlayUrl, 0,
            originalTitle);
      }
      // }
      return true;
    } catch (JSONException exception) {
      loge(methodName, exception);
    }
    return false;
  }

  private boolean getVolumeParamFromJson(String jsonString, Intent intent,
      Context context) {

    String methodName = "getVolumeParamFromJson";
    String channelData = null;
    int volumeData = 0;
    JSONObject jsonObject;

    try {
      jsonObject = (JSONObject) new JSONTokener(jsonString).nextValue();
      if (jsonObject == null) {
        return false;
      }

      // add for DLNA Play begin
      DMRSetVolumeJSON setVolumeJSON = new DMRSetVolumeJSON();
      setVolumeJSON = (DMRSetVolumeJSON) JSONUtils.loadObjectFromJSON(
          jsonString, setVolumeJSON);
      if (null == setVolumeJSON) {
        Log.d(TAG, "setVolumeJSON is null");
        return false;
      }

      if (jsonObject.has(CHANNEL)) {
        channelData = jsonObject.getString(CHANNEL);
        Log.d(TAG, "the volume channelData = " + channelData);
      }
      if (channelData == null) {
        return false;
      }

      if (jsonObject.has(VOLUME)) {
        volumeData = (int) (jsonObject.getDouble(VOLUME) * 100);
        if (ISDEBUG) logd(methodName, "the volumeData = " + volumeData);
      }

      if (DLNAConstant.Channel.STEREO.equals(channelData)
          || channelData.equals("")) {
        if (ISDEBUG) logd(methodName, "mVolume = " + volumeData);
        SoundUtils.setVolumePercent(context, volumeData);
        // add for DLNA PLAY begin
      } else if (DLNAConstant.Channel.LEFT_FRONT.equals(channelData)) {
        DMRSharingInformation.sLeftVolume = setVolumeJSON.getVolume() * 0.01f;
        if (ISDEBUG) logd(methodName, "sLeftVolume = "
            + DMRSharingInformation.sLeftVolume);
        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
            PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME);
        context.sendBroadcast(intent);
      } else if (DLNAConstant.Channel.RIGHT_FRONT.equals(channelData)) {
        DMRSharingInformation.sRightVolume = setVolumeJSON.getVolume() * 0.01f;
        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
            PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME);
        context.sendBroadcast(intent);
      }
    } catch (JSONException exception) {
      loge(methodName, exception);
    }
    return false;
  }

  private boolean getMuteParamFromJson(String jsonString, Context context) {

    DMRSetMuteJSON setMuteJSON = new DMRSetMuteJSON();
    setMuteJSON = (DMRSetMuteJSON) JSONUtils.loadObjectFromJSON(jsonString,
        setMuteJSON);
    if (null == setMuteJSON) {
      Log.d(TAG, "setMuteJSON is null");
      return false;
    }
    String setMuteString = setMuteJSON.getMute();
    if (TextUtils.isEmpty(setMuteString)) {
      Log.d(TAG, "setMuteString is empty");
      return false;
    }
    if (PublicConstants.DLNACommon.SET_MUTE_CONFIRM.equals(setMuteString)) {
      SoundUtils.setMute(context, true);
      return true;
    } else if (PublicConstants.DLNACommon.SET_MUTE_CANCEL.equals(setMuteString)) {
      SoundUtils.setMute(context, false);
      return true;
    }
    return false;
  }

  private void startImagePlayer(Context context, String filePath,
      String originalTitle, boolean isSlideShow) {
    Intent intent3 = new Intent();
    intent3.setClassName("com.hybroad.media", "com.hybroad.image.ImagePlayer");
    intent3.putExtra(PublicConstants.DMRIntent.KEY_URL, filePath);
    intent3.putExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE,
        originalTitle);
    intent3.putExtra(PublicConstants.DMRIntent.KEY_PIC_SLIDESHOW, isSlideShow);
    intent3.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    Log.d(TAG, "startImagePlayer: + filePath = " + filePath);
    context.startActivity(intent3);
  }

  private void startMusicPlayer(Context context, String filePath,
      int seekValue, String originalTitle) {
    Intent intent2 = new Intent();
    intent2.setClassName("com.hybroad.media", "com.hybroad.music.MusicPlayer");
    intent2.putExtra(PublicConstants.DMRIntent.KEY_URL, filePath);
    intent2.putExtra(PublicConstants.DMRIntent.KEY_SEEK_VALUE, seekValue);
    intent2.putExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE,
        originalTitle);
    intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    context.startActivity(intent2);
  }

  private void startVideoPlayer(Context context, String jsonStr) {
    Intent intent = new Intent();
    intent.setClassName("com.hybroad.media", "com.hybroad.video.VideoPlayer");
    intent.putExtra(PublicConstants.DMRIntent.KEY_PLAYJSON, jsonStr);
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    if (mAirPlayClient != null) {
      if (ISDEBUG) logd("startVideoPlayer", "audioTrack.stop()");
      mAirPlayClient.audioDestroy("stop");
    }
    context.startActivity(intent);
  }



  public class AirPlayServiceBinder extends IAirPlayService.Stub {
    @Override
    public void announceToClients(String returnStr) throws RemoteException {
      if (mAirPlayClient != null) {
        if (ISDEBUG) logd("announceToClients:", "returnStr = " + returnStr);
        mAirPlayClient.announceToClients(returnStr);
      }
    }

    @Override
    public void registerVideoPlayerCallback(IVideoPlayerCallback callback)
        throws RemoteException {
      mCallback = callback;
    }
  }

  @Override
  public IBinder onBind(Intent intent) {

    if (mAirPlayServiceBinder == null) {
      mAirPlayServiceBinder = new AirPlayServiceBinder();
    }
    return mAirPlayServiceBinder;
  }

  @Override
  public boolean onUnbind(Intent intent) {
    if (ISDEBUG ) logd("onUnbind", "onUnbind the service");
    mCallback = null;
    return super.onUnbind(intent);
  }

  private static void logd(String methodName, String tag) {
    Log.d(methodName, "[" + tag  + "]");
  }

  private static void loge(String methodName, Exception ex) {
    Log.e(TAG, methodName, ex);
  }
}
