package com.hybroad.util;

import android.os.Environment;
import android.view.KeyEvent;

public class PublicConstants {
  public static final String SELECTED_ITEM_POSITION = "selectedItemPosition";
  public static final String MEDIA_FILE_LIST = "MediaFileList";

  public static final int IR_KEY_AUDIO_CHANNEL = 1185;
  public static final int IR_KEY_RED = KeyEvent.KEYCODE_F2;
  public static final int IR_KEY_GREEN = KeyEvent.KEYCODE_F1;

  // The sort id must be consistent with the corresponding element position in
  // R.array.sort_option_items
  public static final int SORT_BY_NAME = 0;
  public static final int SORT_BY_DATE = 1;
  public static final int SORT_BY_SIZE = 2;
  public static final int SORT_ARRAY_LENGTH = 3;

  public static final class MusicCommon {
    public static final String MUSIC_PLAY_SERVICE = "com.hybroad.music.MUSIC_SERVICE";
    public static final String INTENT_ACTION_PLAY_CMD = "com.hybroad.music.PLAY_CMD";
    public static final String INTENT_ACTION_FLUSH_TIME = "com.hybroad.music.FLUSH_TIME";
    public static final String INTENT_ACTION_DURATION = "com.hybroad.music.MusicDuration";
    public static final String INTENT_KEY_DURATION = "intMusicDuration";
    public static final String INTENT_ACTION_PLAY_ERROR = "com.hybroad.music.PLAY_ERROR";
    public static final String INTENT_ACTION_PLAY_FINISH = "com.hybroad.music.PLAY_FINISH";
    public static final String INTENT_ACTION_PLAY_PREPARED = "com.hybroad.music.PLAY_PREPARED";
    public static final String INTENT_KEY_MUSIC_FILE_PATH = "musicFilePath";
    public static final String INTENT_KEY_SEEK_VALUE = "initSeekPostion";
    // public static final String MUSIC_FAVORITE_PATH =
    // "/mnt/nand/mymedia/music/favorite";
    public static final String MUSIC_FAVORITE_PATH = Environment
        .getExternalStorageDirectory() + "/mymedia/music/favorite";
    public static final int MUSIC_FAVORITE_MAX_NUMBER = 10;
  }

  public static final class DeviceType {
    public static final String LOCAL_DEVICE = "LocalDevice";
    public static final String DMS_DEVICE = "DMSDevice";
  }

  public static final class IntentStarter {
    public static final int NORMAL_START = 0;
    public static final int DMR_START = 1;
    public static final String NORMAL_START_MUSIC_KEY = "com.hybroad.music";
    public static final String NORMAL_START_MUSIC_NAME = "MusicBrowser";
    public static final String NORMAL_START_IMAGE_KEY = "com.hybroad.image";
    public static final String NORMAL_START_IMAGE_NAME = "ImageBrowser";
  }

  public static final class DialogReslut {
    public static final int CONFIRM = 0;
    public static final int CANCEL = 1;
    public static final int NOTHING = 2; // just exit by Pressed the Back
                                         // key
  }

  public static final class DLNACommon {
    public static final String DLNA_BIN_SERVICE_NAME = "dlnaserver";
    public static final String DLNA_BIN_SERVICE_STATE_PROPERTY = "init.svc."
        + DLNA_BIN_SERVICE_NAME;
    public static final String DLNA_BIN_SERVICE_START_KEY = "ro.sys.dlna_start_condition";
    public static final String DLNA_BIN_SERVICE_START_VALUE = "trigger_start_dlnaserver";
    public static final String DMR_JAVA_SERVICE_NAME = "com.hybroad.dlna.DMR_SERVICE";

    public static final String MEDIA_TYPE_VIDEO = "M_VIDEO";
    public static final String MEDIA_TYPE_AUDIO = "M_AUDIO";
    public static final String MEDIA_TYPE_PICTURE = "PICTURE";

    public static final String SET_MUTE_CONFIRM = "1";
    public static final String SET_MUTE_CANCEL = "0";

    public static final String TRANSPORT_STATE_STOPPED = "STOPPED";
    public static final String TRANSPORT_STATE_PAUSED = "PAUSED_PLAYBACK";
    public static final String TRANSPORT_STATE_PLAYING = "PLAYING";
    public static final String TRANSPORT_STATE_TRANSITIONING = "TRANSITIONING";

    public static final String TRANSPORT_STATUS_OK = "OK";
    public static final String TRANSPORT_STATUS_ERROR = "ERROR_OCCURRED";
  }

  /**
   * The following values remain consistent with
   * 'dlna_yuxing/dlna/common/dlna_type.h'
   */
  public static final class DMREvent {
    public static final int DLNA_EVENT_DMR_SETMUTE = 2;
    public static final int DLNA_EVENT_DMR_SETVOLUME = 3;
    public static final int DLNA_EVENT_DMR_GETMUTE = 4;
    public static final int DLNA_EVENT_DMR_GETVOLUME = 5;

    public static final int DLNA_EVENT_DMR_PLAY = 6;
    public static final int DLNA_EVENT_DMR_PAUSE = 7;
    public static final int DLNA_EVENT_DMR_RESUME = 8;
    public static final int DLNA_EVENT_DMR_SEEK = 9;
    public static final int DLNA_EVENT_DMR_STOP = 10;
    public static final int DLNA_EVENT_DMR_EXIT = 0; // only for android
                                                     // dlna

    public static final int DLNA_EVENT_DMR_GETPOSITIONINFO = 11;
    public static final int DLNA_EVENT_DMR_GETTRANSPORTINFO = 12;
    public static final int DLNA_EVENT_DMR_GETMEDIAINFO = 13;

    public static final int DLNA_EVENT_DMR_NONE = 14;

    public static final int DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM = 16;
    public static final int DLNA_EVENT_DMR_GETPRODUCTINFO = 17;
    public static final int DLNA_EVENT_DMR_ORDER = 18;

  }

  /*
   * typedef enum { DLNA_EVENT_DMSLIST_UPDATE = 0, DMS list refresh
   * DLNA_EVENT_FILELIST_UPDATE, file list updata DLNA_EVENT_DMR_SETMUTE, 2 set
   * mute DLNA_EVENT_DMR_SETVOLUME, 3 set volume DLNA_EVENT_DMR_GETMUTE, 4 get
   * mute status DLNA_EVENT_DMR_GETVOLUME, 5 get volume DLNA_EVENT_DMR_PLAY, 6
   * play DLNA_EVENT_DMR_PAUSE, 7 pause DLNA_EVENT_DMR_RESUME, 8 resume
   * DLNA_EVENT_DMR_SEEK, 9 seek to DLNA_EVENT_DMR_STOP, 10 stop
   * DLNA_EVENT_DMR_GETPOSITIONINFO, 11 get position info
   * DLNA_EVENT_DMR_GETTRANSPORTINFO, 12 get transport info
   * DLNA_EVENT_DMR_GETMEDIAINFO, 13 get media info DLNA_EVENT_DMR_NONE, 14 in
   * vain EVENT_DLNA_CHANNEL_RESOURCE_CHANGED, 15
   * DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM, 16\ DLNA_EVENT_DMR_GETPRODUCTINFO,17\
   * DLNA_EVENT_DMR_ORDER \\ order
   * 
   * }HY_DLNA_EVENTID;
   */

  public static final class DMRIntent {
    public static final String ACTION_START_DMR = "com.hybroad.dlna.START_DMR";
    public static final String ACTION_DMR_EVENT = "com.hybroad.dlna.DMR_EVENT";
    public static final String ACTION_DLNA_SERVER_DIED = "com.hybroad.dlna.SERVER_DIED";
    public static final String ACTION_DLNA_SERVER_STARTED = "com.hybroad.dlna.SERVER_STARTED";

    public static final String KEY_URL = "com.hybroad.dmr.url";
    public static final String KEY_SEEK_VALUE = "com.hybroad.dmr.seek";
    public static final String KEY_PLAY_CMD = "com.hybroad.dmr.playCMD";
    public static final String KEY_ORIGINAL_TITLE = "com.hybroad.dmr.originalTitle";
    // add liumeidong 20131115 begin
    public static final String KEY_PLAYJSON = "com.hybroad.dmr.playJsonStr";
    // add liumeidong 20131115 end
    // add 20131206 for the airPlay pic play begin
    public static final String KEY_PIC_SLIDESHOW = "com.hybroad.airplay.slideshow";
    // add 20131206 for the airPlay pic play end

  }

  public static final class DMPIntent {
    public static final String ACTION_DMS_DEVICE_CHANGE = "com.hybroad.dms.change.device";
    public static final String ACTION_DMS_FILE_CHANGE = "com.hybroad.dms.change.file";
  }

  public static final class ActivityState {
    public static final int UNKNOWN = -1;
    public static final int ON_CREATE = 0;
    public static final int ON_NEWINTENT = 1;
    public static final int ON_RESTART = 2;
    public static final int ON_START = 3;
    public static final int ON_RESUME = 4;
    public static final int ON_PAUSE = 5;
    public static final int ON_STOP = 6;
    public static final int ON_DESTROY = 7;
  }

  public static final class NetworkState {
    public static final int UNKNOWN = -1;
    public static final int DISCONNECTED = 0;
    public static final int ETHERNET_CONNECTED = 1;
    public static final int WIFI_1 = 2;
    public static final int WIFI_2 = 3;
    public static final int WIFI_3 = 4;
    public static final int WIFI_FULL = 5;
  }
}
