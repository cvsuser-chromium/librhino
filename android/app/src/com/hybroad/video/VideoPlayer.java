package com.hybroad.video;

import java.util.Iterator;
import java.util.LinkedHashMap;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hybroad.dlna.DMPMessageTransit;
import com.hybroad.dlna.DMRSharingInformation;
import com.hybroad.dlna.OnCurrentDMSChanged;
import com.hybroad.dlna.aidl.IAirPlayService;
import com.hybroad.dlna.aidl.IVideoPlayerCallback;
import com.hybroad.media.DevicesList;
import com.hybroad.media.R;
import com.hybroad.util.Converter;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.PublicConstants.DialogReslut;
import com.hybroad.util.PublicConstants.IntentStarter;
import com.hybroad.util.SharedPreferencesHelper;

public class VideoPlayer extends Activity {
  private final static String TAG = "VideoPlayer";
  private Context mContext = VideoPlayer.this;
  private Activity mActivity = VideoPlayer.this;

  private static int mScreenWidth = 0;
  private static int mScreenHeight = 0;
  private VideoView mVideoView = null;
  private SurfaceView mSubVideoView = null;
  private SurfaceHolder mSubHolder;
  private VideoPlayList mVideoList = null;

  private String mCurrentVideoPath;
  private String mDMRVideoPath;
  private LinkedHashMap<String, String> mHeaders;
  private int mDuration = 0;
  private int mSeekStep = 60;
  private int mRewindRate = 1;
  private int mForwardRate = 1;
  private int mPlayMode = VideoConstant.PLAY_ALL_CYCLE;

  private View mMediaControllerLayout;
  private SeekBar mVideoSeekBar;
  private ImageButton mPlayButton;
  private TextView mCurrentTimeText;
  private TextView mForwardRewindText;
  private int mSeekBarMax = 1000;

  private long mPreHideStartTime = 0;
  private Handler mAutoHideHandler = null;
  private AutoHideRunnable mAutoHideRunnable = null;

  private View mVideoMenuLayout;
  private View mVideoPlayModeMenuLayout;
  private ImageView mVideoPlayModeMenuIcon;
  private TextView mVideoPlayModeMenuTitle;
  private View mFullScreenContrlMenuLayout;
  private TextView mFullScreenContrlTitle;
  private View mSwitchSubtitleMenuLayout;
  private View mSwitchTrackMenuLayout;
  private View mSwitchChannelMenuLayout;
  private int mCurrentTrackID = 0;
  private int mCurrentSubtitleID = 0;
  private int mCurrentChannelPostion = 0;
  private String[] mAudioChannels = null;

  private boolean isFullScreen = true;
  private boolean isStop = false;
  private boolean isAutoHideThreadStart = false;
  private boolean isSeeking = false;
  private boolean isRewindOrForward = false;

  /* 3D mode flag, default value is 0, indicate 2D mode. */
  private int mType = 0;
  private boolean isEnded = false;

  private int mPlayIntentType = IntentStarter.NORMAL_START;
  private int mDMRDialogResult = DialogReslut.NOTHING;
  private StorageManager mStorageManager = null;
  private DMPMessageTransit mDMPMessageTransit;
  private DMRMessageReceiver mDMRMessageReceiver;
  public String mCommunicateStr = null;
  private IAirPlayService mAirPlayService;

  private static final String HEADER = "Header";
  private static final String STARTPERCENT = "StartPercent";
  private static final String PLAY_URL = "PlayUrl";
  private double mStartMillsec = 0.0d;
  private static final boolean ISDEBUG = true;
  private ProgressDialog progressDialog;
  private boolean mFlagStopTimer = false;
  private String mJsonString = null;

  public void onCreate(Bundle savedInstanceState) {
    String methodName = "onCreate";
    if (ISDEBUG) logd(methodName, "");
    super.onCreate(savedInstanceState);
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    setContentView(R.layout.video_player_main);

    mDMRMessageReceiver = new DMRMessageReceiver();
    IntentFilter mDMRFilter = new IntentFilter();
    mDMRFilter.addAction(PublicConstants.DMRIntent.ACTION_DMR_EVENT);
    registerReceiver(mDMRMessageReceiver, mDMRFilter);

    mHeaders = new LinkedHashMap<String, String>();
    CommonData.isLastVideoFile = false;
    isEnded = false;

    init();

    Intent intent = getIntent();
    mVideoList = intent.getParcelableExtra(PublicConstants.MEDIA_FILE_LIST);
    if (mVideoList != null) {
      mPlayIntentType = IntentStarter.NORMAL_START;
      VideoFile videoModel = mVideoList.getCurrentVideo();
      if (null == videoModel) {
        finish();
      }
      if (videoModel != null) {
        mCurrentVideoPath = videoModel.path;
      }
      if (TextUtils.isEmpty(mCurrentVideoPath)) {
        finish();
      }
      mVideoView.setVideoPath(mCurrentVideoPath);
    } else {
      mPlayIntentType = IntentStarter.DMR_START;
      //mDMRVideoPath = intent.getStringExtra(PublicConstants.DMRIntent.KEY_URL);
      mJsonString = intent
          .getStringExtra(PublicConstants.DMRIntent.KEY_PLAYJSON);
      getPlayParams(mJsonString);

      // add liumeidong 20131115 begin
      if (TextUtils.isEmpty(mDMRVideoPath)) {
        finish();
      }
      // add liumeidong 20131115 end
      // popDMRRequestDialogOnCreate();
      mCurrentVideoPath = mDMRVideoPath;
      // new
      if (ISDEBUG) logd(methodName, " mHeaders = " + mHeaders + ", mCurrentVideoPath = "
          + mCurrentVideoPath);
      mVideoView.setVideoPath(mCurrentVideoPath, mHeaders);

      startUpdateDMRInformation();
    }
    // add liumeidong 20131107 begin
    Intent airPlayIntent = new Intent("com.hybroad.dlna.AIRPLAY_SERVICE");
    mContext.bindService(airPlayIntent, mConnection, Context.BIND_AUTO_CREATE);
    // add liumeidong 20131107 end
    progressDialog = new ProgressDialog(mContext, 0);
    progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
  }

  private void getPlayParams(String jsonString) {

    String methodName = "getVideoPlayParams";
    JSONObject jsonObject;
    // key ,value is the video play mHead params
    String key;
    String value;

    if (jsonString == null && TextUtils.isEmpty(jsonString)) {
      return;
    }

    try {
      jsonObject = (JSONObject) new JSONTokener(jsonString).nextValue();
      if (jsonObject == null) {
        return;
      }
      if (jsonObject.has(HEADER)) {
        JSONObject headJsonObject = jsonObject.getJSONObject(HEADER);
        if (headJsonObject != null) {
          Iterator<?> iterator = headJsonObject.keys();
          while (iterator.hasNext()) {
            key = (String) iterator.next();
            value = headJsonObject.getString(key);
            if (ISDEBUG) logd(methodName, "key =" + key + ", value = " + value);
            if (!TextUtils.isEmpty(key) && !TextUtils.isEmpty(value)) {
              mHeaders.put(key, value);
              if (ISDEBUG) logd(methodName, "mHeaders = " + mHeaders);
            }
          }
        }
      }

      // get the video first play start percent
      if (jsonObject.has(STARTPERCENT)) {
        mStartMillsec = jsonObject.getDouble(STARTPERCENT);
        if (ISDEBUG) logd(methodName, " mStartMillsec = " + mStartMillsec);
      }

      if (jsonObject.has(PLAY_URL)) {
        mDMRVideoPath = jsonObject.getString(PLAY_URL);
        logd(methodName, "mDMRVideoPath =" + mDMRVideoPath);
      }
    } catch (JSONException jsonException) {
      loge(methodName, jsonException);
    }
  }

  // add liumeidong 201311181513 end

  // add 20131107 liumeidong begin
  private ServiceConnection mConnection = new ServiceConnection() {

    public void onServiceConnected(ComponentName componentName, IBinder service) {
      String methodName = "onServiceConnected";
      if (ISDEBUG) logd(methodName, "componentName = " + componentName);
      mAirPlayService = IAirPlayService.Stub.asInterface(service);
      try {
        mAirPlayService.registerVideoPlayerCallback(mCallback);
        if (mAirPlayService != null) {
          String returnJson = "{" + "\"sender\": \"xbmc\","
              + "\"status\": \"OnPlay\"}";
          if (ISDEBUG) logd(methodName, "returnJson = " + returnJson);
            mAirPlayService.announceToClients(returnJson);
          }
      } catch (RemoteException exception) {
        loge(methodName, exception);
      }
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
      String methodName = "onServiceDisconnected";
      if (ISDEBUG) logd(methodName, "mAirPlayService = " + mAirPlayService);
      if (mAirPlayService != null) {
        mAirPlayService = null;
      }
    };
  };

  private IVideoPlayerCallback mCallback = new IVideoPlayerCallback.Stub() {

    @Override
    public String getCurrentPositionInfo() throws RemoteException {
      String returnJson = "{ " + "\"Position\": " + mCurrentPlayPostion + ", "
          + "\"Duration\": " + DMRSharingInformation.sDuration + ", "
          + "\"returncode\": \"0\", " + "\"Track\": \"0\", "
          + "\"RelCount\": \"0\", " + "\"AbsCount\": \"0\", "
          + "\"TrackURI\": \"" + mDMRVideoPath + "\"" + " }";

      if (ISDEBUG) logd("getCurrentPositionInfo", returnJson); 
      return returnJson;
    }
  };

  // add 20131107 liumeidong end

  protected void onNewIntent(Intent intent) {
    String methodName = "onNewIntent";
    if (ISDEBUG) logd(methodName, "intent = " + intent);
    //setIntent(intent); // must store the new intent otherwise getIntent()
                       // will return the old one

    //mDMRVideoPath = intent.getStringExtra(PublicConstants.DMRIntent.KEY_URL);
    
    mJsonString = intent.getStringExtra(PublicConstants.DMRIntent.KEY_PLAYJSON);
    getPlayParams(mJsonString);
    if (TextUtils.isEmpty(mDMRVideoPath)) {
      return; // do nothing
    }

    // popDMRRequestDialogOnNewIntent();
    mPlayIntentType = IntentStarter.DMR_START;
    mCurrentVideoPath = mDMRVideoPath;
    mVideoView.setVideoPath(mCurrentVideoPath, mHeaders);
    startUpdateDMRInformation();
  }

  protected void onRestart() {
    logd("onRestart", "");
    // add liumeidong 20131107 begin
    // Intent airPlayIntent = new
    // Intent("com.hybroad.dlna.AIRPLAY_SERVICE");
    // mContext.bindService(airPlayIntent, mConnection,
    // Context.BIND_AUTO_CREATE);
    // add liumeidong 20131107 end
    super.onRestart();
  }

  protected void onStart() {
    if (ISDEBUG) logd("onStart", "");
    super.onStart();

    if (null != DevicesList.s_currentDevice
        && DevicesList.s_currentDevice.isLocalDevice()) {
      if (mStorageManager == null) {
        mStorageManager = (StorageManager) mActivity
            .getSystemService(STORAGE_SERVICE);
      }
      mStorageManager.registerListener(mStorageEventListener);
    } else if (null != DevicesList.s_currentDevice
        && DevicesList.s_currentDevice.isDMSDevice()) {
      if (null == mDMPMessageTransit) {
        mDMPMessageTransit = new DMPMessageTransit(new OnCurrentDMSChanged() {
          @Override
          public void onDMSDeviceDown() {
            CustomWidget.toastDeviceDown(mContext);
            finish();
          }
        });
      }
      IntentFilter filter = new IntentFilter(
          PublicConstants.DMPIntent.ACTION_DMS_DEVICE_CHANGE);
      registerReceiver(mDMPMessageTransit, filter);
    }
  }

  protected void onResume() {
    String methodName = "onResume";
    if (ISDEBUG) logd(methodName, "");
    Window window = getWindow();
    View decorView = getWindow().getDecorView();
    View content = findViewById(R.id.m_content);

    // hide the system bar
    int uiOptions = content.getSystemUiVisibility();
    uiOptions = uiOptions & ~View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
    uiOptions = uiOptions & ~View.SYSTEM_UI_FLAG_FULLSCREEN;
    content.setSystemUiVisibility(uiOptions);
    ActionBar actionBar = getActionBar();
    if (ISDEBUG) logd(methodName, "actionBar = " + actionBar);
    // actionBar.hide();
    super.onResume();
  }

  protected void onPause() {
    logd("onPause", "");
    // if (SystemProperties.get("chip.type").equals("hi3716c")) {
    // synchronized (this) {
    // mVideoView.set3DMode(0);
    // mVideoView.setStereoMode(0);
    // mType = 0;
    // isEnded = true;
    // }
    // }
    stopUpdateDMRInformation();
    super.onPause();
  }

  protected void onStop() {
    String methodName = "onStop";
    if (ISDEBUG) logd(methodName, "");
    // add 20131106 begin
    if (mAirPlayService != null) {
      String returnJson = "{ " + "\"sender\":\"xbmc\","
          + "\"status\":\"OnStop\" }";
      if (ISDEBUG) logd(methodName, "returnJson = " + returnJson);
      try {
        mAirPlayService.announceToClients(returnJson);
      } catch (RemoteException e) {
        loge(methodName, e);
      }
    }
    // add liumeidong 20131107 begin
    if (mConnection != null) {
      unbindService(mConnection);
    }
    // add liumeidong 20131107 end
    // add 20131106 end
    // this.isStop = true;
    super.onStop();

    if (null != mStorageManager) {
      mStorageManager.unregisterListener(mStorageEventListener);
    }
    if (null != mDMPMessageTransit) {
      unregisterReceiver(mDMPMessageTransit);
    }
  }

  protected void onDestroy() {
    logd("onDestroy", "");
    if (mVideoView != null) {
      mVideoView.destroyPlayer();
    }
    if (null != mAutoHideHandler && null != mAutoHideRunnable)
      mAutoHideHandler.removeCallbacks(mAutoHideRunnable);

    stopUpdateDMRInformation();
    DMRSharingInformation.resetDMRShareData();
    if (null != mDMRMessageReceiver) {
      unregisterReceiver(mDMRMessageReceiver);
    }

    if (mHeaders != null) {
      mHeaders = null;
    }

    if (mAirPlayService != null) {
      try {
        mAirPlayService.registerVideoPlayerCallback(null);
      } catch (RemoteException e) {
        e.printStackTrace();
      }
      mAirPlayService = null;
    }
    super.onDestroy();
  }

  private void popDMRRequestDialogOnCreate() {
    Dialog playDMRDialog = new AlertDialog.Builder(this)
        .setIcon(android.R.drawable.ic_dialog_info)
        .setTitle(getString(R.string.dmr_play_request))
        .setMessage(
            getString(R.string.dmr_play_request_content) + mDMRVideoPath)
        .setPositiveButton(getString(R.string.confirm),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                mDMRDialogResult = DialogReslut.CONFIRM;
                dialog.dismiss();
                mCurrentVideoPath = mDMRVideoPath;

                mVideoView.setVideoPath(mCurrentVideoPath);
              }
            })
        .setNegativeButton(getString(R.string.cancel),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                dialog.dismiss();
                if (mDMRDialogResult != DialogReslut.CONFIRM) // used
                                                              // to
                                                              // continuous
                                                              // repeated
                                                              // requests
                                                              // pending
                                                              // case
                  finish();
              }
            }).show();
    playDMRDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
      @Override
      public void onDismiss(DialogInterface arg0) {
        if (mDMRDialogResult == DialogReslut.NOTHING)
          finish();
      }
    });
    WindowManager.LayoutParams params = playDMRDialog.getWindow()
        .getAttributes();
    params.width = 400;
    playDMRDialog.getWindow().setAttributes(params);
  }

  private void popDMRRequestDialogOnNewIntent() {
    Dialog playDMRDialog = new AlertDialog.Builder(this)
        .setIcon(android.R.drawable.ic_dialog_info)
        .setTitle(getString(R.string.dmr_play_request))
        .setMessage(
            getString(R.string.dmr_play_request_content) + mDMRVideoPath)
        .setPositiveButton(getString(R.string.confirm),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                mDMRDialogResult = DialogReslut.CONFIRM; // used
                                                         // to
                                                         // continuous
                                                         // repeated
                                                         // requests
                                                         // pending
                                                         // case
                dialog.dismiss();
                mPlayIntentType = IntentStarter.DMR_START;
                mCurrentVideoPath = mDMRVideoPath;
                mVideoView.setVideoPath(mCurrentVideoPath);
              }
            })
        .setNegativeButton(getString(R.string.cancel),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                dialog.dismiss();
              }
            }).show();
    WindowManager.LayoutParams params = playDMRDialog.getWindow()
        .getAttributes();
    params.width = 400;
    playDMRDialog.getWindow().setAttributes(params);
  }

  public boolean onTouchEvent(MotionEvent event) {
    if (MotionEvent.ACTION_DOWN == event.getAction()
        || MotionEvent.ACTION_MOVE == event.getAction()) {
      if (isFullScreen) {
        mMediaControllerLayout.setVisibility(View.VISIBLE);
      }
      startAutoHideThread();
      return false;
    }
    return super.onTouchEvent(event);
  }

  private void init() {
    getScreenSize();
    mAutoHideHandler = new Handler();
    mAutoHideRunnable = new AutoHideRunnable();

    canRunInThread();

    View content = findViewById(R.id.m_content);
    mVideoView = (VideoView) findViewById(R.id.videoView);
    mSubVideoView = (SurfaceView) findViewById(R.id.subVideoView);
    mSubHolder = mSubVideoView.getHolder();
    mSubHolder.setType(SurfaceHolder.SURFACE_TYPE_NORMAL);
    mSubVideoView.setZOrderOnTop(true);
    mSubHolder.setFormat(PixelFormat.TRANSLUCENT);
    mVideoSeekBar = (SeekBar) findViewById(R.id.videoSeekBar);
    mCurrentTimeText = (TextView) findViewById(R.id.timeText);

    mMediaControllerLayout = findViewById(R.id.mediaControllerLayout);
    mMediaControllerLayout.getBackground().setAlpha(0);
    // mMediaControllerLayout.setVisibility(View.INVISIBLE);
    mMediaControllerLayout.setVisibility(View.GONE);

    mVideoMenuLayout = this.findViewById(R.id.video_menu_layout_content);
    mVideoPlayModeMenuLayout = this
        .findViewById(R.id.video_play_mode_menu_layout);
    mVideoPlayModeMenuIcon = (ImageView) this
        .findViewById(R.id.video_play_mode_menu_icon);
    mVideoPlayModeMenuTitle = (TextView) this
        .findViewById(R.id.video_play_mode_menu_title);
    mFullScreenContrlMenuLayout = this
        .findViewById(R.id.full_screen_control_menu_layout);
    mFullScreenContrlTitle = (TextView) this
        .findViewById(R.id.full_screen_control_menu_title);
    mSwitchSubtitleMenuLayout = this
        .findViewById(R.id.switch_subtitle_menu_layout);
    mSwitchTrackMenuLayout = this
        .findViewById(R.id.switch_audio_track_menu_layout);
    mSwitchChannelMenuLayout = this
        .findViewById(R.id.switch_channel_menu_layout);

    setVideoScale(VideoConstant.SCREEN_FULL);

    mVideoSeekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);
    mVideoView.setOnPreparedListener(mPreparedListener);
    mVideoView.setOnCompletionListener(mCompletionListener);
    mVideoView.setOnErrorListener(mOnErrorListener);
    // mVideoView
    // .setOnFastBackwordCompleteListener(mFastBackwordCompleteListener);

    // if (SystemProperties.get("chip.type").equals("hi3716c")) {
    mVideoView.setOn3DModeReceivedListener(mOnInfoListener);
    // }
  }

  private void canRunInThread() {
    mPlayMode = SharedPreferencesHelper.getValue(mContext,
        SharedPreferencesHelper.XML_VIDEO_SETTINGS,
        SharedPreferencesHelper.KEY_VIDEO_PLAY_MODE,
        VideoConstant.PLAY_ALL_CYCLE);

    mForwardRewindText = (TextView) findViewById(R.id.forwardRewindTextCue);

    mPlayButton = (ImageButton) findViewById(R.id.play_pause);
    mPlayButton.setOnClickListener(new ButtonClickListener());
  }

  final StorageEventListener mStorageEventListener = new StorageEventListener() {
    @Override
    public void onStorageStateChanged(String path, String oldState,
        String newState) {
      super.onStorageStateChanged(path, oldState, newState);
      if (newState.equals("unmounted")) {
        if (DevicesList.s_currentDevice != null && path != null
            && path.equals(DevicesList.s_currentDevice.getRootPath())) {
          finish();
        }
      }
    }
  };

  public class ButtonClickListener implements View.OnClickListener {
    public void onClick(View v) {
      if (v == mPlayButton) {
        play_pause();
      }

      if (isFullScreen) {
        startAutoHideThread();
      }
    }
  }

  SeekBar.OnSeekBarChangeListener mSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
    public void onStopTrackingTouch(SeekBar seekBar) {
      startAutoHideThread();
    }

    public void onStartTrackingTouch(SeekBar seekBar) {
      startAutoHideThread();
    }

    public void onProgressChanged(SeekBar seekBar, int progress,
        boolean fromUser) {
      if (fromUser) {
        mVideoView.seekTo((int) (progress * 1.0 / seekBar.getMax() * mVideoView
            .getDuration()));
        startAutoHideThread();
      }
    }
  };

  MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
    public void onPrepared(MediaPlayer mp) {
      String methodName = "onPrepared";
      doExtraOpration();
      start();
      // add 20131107 begin
      // if (IntentStarter.DMR_START == mPlayIntentType
      // && DMRSharingInformation.sInitSeekPosition > 0) {
      // mp.seekTo(DMRSharingInformation.sInitSeekPosition);
      // }

      try {
        if (mStartMillsec > 0) {
          double startMillsec = mStartMillsec * mp.getDuration() / 100;
          mp.seekTo((int) startMillsec);
          mStartMillsec = 0.0d;
        }
        String returnJson = "{" + "\"sender\" : \"xbmc\","
            + "\"status\" : \"OnPlay\" }";
        if (ISDEBUG) logd(methodName, " returnJson = " + returnJson);
        mAirPlayService.announceToClients(returnJson);
        
      } catch (RemoteException e) {
        loge(methodName, e);
      }
      // add 20131107 end
    }
  };

  MediaPlayer.OnCompletionListener mCompletionListener = new MediaPlayer.OnCompletionListener() {
    public void onCompletion(MediaPlayer mp) {
      String methodName = "onCompletion";
      if (ISDEBUG) logd(methodName, "mp = " + mp);
      mCurrentTrackID = 0;
      mCurrentSubtitleID = 0;
      mCurrentChannelPostion = 0;
      DMRSharingInformation.resetDMRShareData();
      mp.setLooping(false);
      if (mPlayMode == VideoConstant.PLAY_IN_ORDER) {
        /* Play in order */
        if (mVideoList != null) {
          VideoFile videoModel = mVideoList.getNextVideo_NoCycle();
          if (null == videoModel || CommonData.isLastVideoFile) {
            finish();
          } else {
            mCurrentVideoPath = videoModel.path;
            mVideoView.setVideoPath(mCurrentVideoPath);
          }
        } else {
          finish();
        }
      } else if (mPlayMode == VideoConstant.PLAY_ALL_CYCLE) {
        /* All loop */
        if (mVideoList != null) {
          VideoFile videoModel = mVideoList.getNextVideo();
          if (null == videoModel) {
            finish();
          } else {
            mCurrentVideoPath = videoModel.path;
            mVideoView.setVideoPath(mCurrentVideoPath);
          }
        } else {
          finish();
        }
      } else if (mPlayMode == VideoConstant.PLAY_ONE_CYCLE) {
        /* Single loop */
        mp.setLooping(true);
        mVideoView.setVideoPath(mCurrentVideoPath);
      } else if (mPlayMode == VideoConstant.PLAY_ONE_SINGLE_TIME) {
        /* Single mPlayButton */
        finish();
      } else if (mPlayMode == VideoConstant.PLAY_RANDOM) {
        /* Random mPlayButton */
        if (mVideoList != null) {
          VideoFile videoModel = mVideoList.getRandomVideo();
          if (null == videoModel) {
            finish();
          } else {
            mCurrentVideoPath = videoModel.path;
            mVideoView.setVideoPath(mCurrentVideoPath);
          }
        } else {
          finish();
        }
      }

      // if (SystemProperties.get("chip.type").equals("hi3716c")) {
      // mVideoView.set3DMode(0);
      // mVideoView.setStereoMode(0);
      // mType = 0;
      // }
    }
  };

  MediaPlayer.OnInfoListener mOnInfoListener = new MediaPlayer.OnInfoListener() {
    public boolean onInfo(MediaPlayer mp, int what, final int extra) {
      String methodName = "mOnInfoListener.onInfo";
      int BufferSize = mVideoView.getBufferPercentage();
      if (ISDEBUG) logd(methodName, ", what = " + what + ", BufferSize = "
          + BufferSize);
      if (!mFlagStopTimer) {
        if (ISDEBUG) logd(methodName, "mFlagStopTimer = " + mFlagStopTimer);
        progressDialog.setMessage(BufferSize + "%");
        // progressDialog.show();
      }
      if (what == 1008) {
        progressDialog.dismiss();
        mFlagStopTimer = true;
      }

      if (what == 1011) {
        if (ISDEBUG) logd(methodName, "get 3D mode: " + extra);
        mFlagStopTimer = true;
        progressDialog.dismiss();
        if (mType == 0 && extra > 0) {
          if (ISDEBUG) logd(methodName, "mType = " + mType + "extra = " + extra 
              + ",isEnded = " + isEnded);
          if (!isEnded) {
            synchronized (this) {
              mVideoView.set3DMode(extra);
              mVideoView.setStereoMode(extra);
            }
          }
        }
        mType = extra;
      }
      return false;
    }
  };

  // OnFastBackwordCompleteListener mFastBackwordCompleteListener = new
  // OnFastBackwordCompleteListener() {
  // public void onFastBackwordComplete(MediaPlayer mp) {
  // Log.i(TAG, "in OnFastBackwordCompleteListener");
  // mPlayButton.setBackgroundResource(R.drawable.pause);
  // mForwardRewindText.setVisibility(View.INVISIBLE);
  // mForwardRate = 1;
  // mRewindRate = 1;
  // isRewindOrForward = false;
  // }
  // };

  MediaPlayer.OnErrorListener mOnErrorListener = new MediaPlayer.OnErrorListener() {
    public boolean onError(MediaPlayer mp, int what, int extra) {
      int messageId = 0;
      
      if (ISDEBUG) logd(TAG, "in the MediaPlayer.OnErrorListener----what:" + what
          + " extra:" + extra + " mediaplayer:" + mp);
      switch (what) {
      case MediaPlayer.MEDIA_ERROR_SERVER_DIED:
        messageId = R.string.media_server_died;
        break;

      case MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK:
        messageId = R.string.not_valid_for_progressive_playback;
        break;

      case MediaPlayer.MEDIA_ERROR_UNKNOWN:
      default:
        messageId = R.string.unknown_error;
        break;
      }
      notSupportDialog(messageId);
      return false;
    }
  };

  private void doExtraOpration() {
    /* Duration of video */
    String methodName = "doExtraOpration";
    mDuration = mVideoView.getDuration();
    DMRSharingInformation.sDuration = String.valueOf(mDuration / 1000);
    if (ISDEBUG) logd(methodName, "Duration =" + (long) mDuration);
    TextView totalTV = (TextView) findViewById(R.id.timetotal);
    if (mDuration > 0) {
      totalTV.setText(Converter.convertToHoursMinutesSeconds(mDuration));
    } else {
      totalTV.setText(R.string.nototaltime);
    }
    if (mDuration < 600000) { // if mDuration < 10min, set step to
                              // 1/10*mDuration
      mSeekStep = mDuration / 10;
    } else {
      mSeekStep = 60000;
    }
  }

  private void notSupportDialog(int msg) {
    AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
    builder.setTitle(com.android.internal.R.string.VideoView_error_title);
    builder.setMessage(msg);
    builder.setPositiveButton(R.string.confirm, null);
    Dialog notSptDialog = builder.create();
    notSptDialog.setOnDismissListener(new OnDismissListener() {
      public void onDismiss(DialogInterface dialog) {
        DMRSharingInformation.resetDMRShareData();
        mActivity.finish();
      }
    });
    notSptDialog.show();
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    String methodName = "onKeyDown";
    if (ISDEBUG) logd(methodName, "keyCode:" + keyCode);

    if (keyCode == KeyEvent.KEYCODE_BACK) {
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        mVideoMenuLayout.setVisibility(View.GONE);
        return true;
      }
      if (isAutoHideThreadStart && isFullScreen) {
        ctrlbarDismiss();
        return true;
      }
      return super.onKeyDown(keyCode, event);
    } else if (keyCode == PublicConstants.IR_KEY_AUDIO_CHANNEL) {
      switchChannel();
      return true;
    }

    if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT
        || keyCode == KeyEvent.KEYCODE_DPAD_LEFT
        || keyCode == KeyEvent.KEYCODE_DPAD_UP
        || keyCode == KeyEvent.KEYCODE_DPAD_DOWN
        || keyCode == KeyEvent.KEYCODE_MEDIA_FAST_FORWARD
        || keyCode == KeyEvent.KEYCODE_MEDIA_REWIND
        || keyCode == KeyEvent.KEYCODE_MEDIA_PREVIOUS
        || keyCode == KeyEvent.KEYCODE_MEDIA_NEXT
        || keyCode == KeyEvent.KEYCODE_ENTER
        || keyCode == KeyEvent.KEYCODE_DPAD_CENTER
        || keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE
        || keyCode == PublicConstants.IR_KEY_RED
        || keyCode == PublicConstants.IR_KEY_GREEN) {
      /* If it is fullscreen, start control thread of control bar */
      if (isFullScreen) {
        if (mVideoMenuLayout == null || !mVideoMenuLayout.isShown()) {
          preUpdateProgressBar();
          mMediaControllerLayout.setVisibility(View.VISIBLE);
          startAutoHideThread();
        }
      }
    }

    switch (keyCode) {
    case KeyEvent.KEYCODE_MENU:
      if (mVideoMenuLayout != null) {
        if (mVideoMenuLayout.isShown()) {
          mVideoMenuLayout.setVisibility(View.GONE);
        } else {
          if (mPlayMode == VideoConstant.PLAY_ALL_CYCLE) {
            mVideoPlayModeMenuIcon.setImageResource(R.drawable.close_auto_play);
            mVideoPlayModeMenuTitle.setText(R.string.closeAutoPlay);
          } else {
            mVideoPlayModeMenuIcon.setImageResource(R.drawable.open_auto_play);
            mVideoPlayModeMenuTitle.setText(R.string.openAutoPlay);
          }
          mVideoMenuLayout.setVisibility(View.VISIBLE);
          mVideoPlayModeMenuLayout.requestFocus();
        }
      }
      return true;
    case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
      fastForwardPlay();
      return true;
    case KeyEvent.KEYCODE_DPAD_RIGHT:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        if (mVideoPlayModeMenuLayout != null
            && mVideoPlayModeMenuLayout.isFocused()
            && mFullScreenContrlMenuLayout != null) {
          mFullScreenContrlMenuLayout.requestFocus();
        } else if (mFullScreenContrlMenuLayout != null
            && mFullScreenContrlMenuLayout.isFocused()
            && mSwitchSubtitleMenuLayout != null) {
          mSwitchSubtitleMenuLayout.requestFocus();
        } else if (mSwitchSubtitleMenuLayout != null
            && mSwitchSubtitleMenuLayout.isFocused()
            && mSwitchTrackMenuLayout != null) {
          mSwitchTrackMenuLayout.requestFocus();
        } else if (mSwitchTrackMenuLayout != null
            && mSwitchTrackMenuLayout.isFocused()
            && mSwitchChannelMenuLayout != null) {
          mSwitchChannelMenuLayout.requestFocus();
        }
        return true;
      }
      seek(true);
      return true;
    case PublicConstants.IR_KEY_GREEN:
      seekTo(mDuration - 3000);
      return true;
    case KeyEvent.KEYCODE_MEDIA_REWIND:
      rewindPlay();
      return true;
    case KeyEvent.KEYCODE_DPAD_LEFT:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        if (mSwitchChannelMenuLayout != null
            && mSwitchChannelMenuLayout.isFocused()
            && mSwitchTrackMenuLayout != null) {
          mSwitchTrackMenuLayout.requestFocus();
        } else if (mSwitchTrackMenuLayout != null
            && mSwitchTrackMenuLayout.isFocused()
            && mSwitchSubtitleMenuLayout != null) {
          mSwitchSubtitleMenuLayout.requestFocus();
        } else if (mSwitchSubtitleMenuLayout != null
            && mSwitchSubtitleMenuLayout.isFocused()
            && mFullScreenContrlMenuLayout != null) {
          mFullScreenContrlMenuLayout.requestFocus();
        } else if (mFullScreenContrlMenuLayout != null
            && mFullScreenContrlMenuLayout.isFocused()
            && mVideoPlayModeMenuLayout != null) {
          mVideoPlayModeMenuLayout.requestFocus();
        }
        return true;
      }
      seek(false);
      return true;
    case PublicConstants.IR_KEY_RED:
      seekTo(0);
      return true;
    case KeyEvent.KEYCODE_MEDIA_STOP:
      DMRSharingInformation.resetDMRShareData();
      finish();
      break;
    case KeyEvent.KEYCODE_MEDIA_NEXT:
    case KeyEvent.KEYCODE_PAGE_DOWN:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        return true;
      }
      mCurrentTrackID = 0;
      mCurrentSubtitleID = 0;
      mCurrentChannelPostion = 0;
      /** start add by yangjie 2011/11/17 */
      if (isRewindOrForward) {
        mForwardRewindText.setVisibility(View.INVISIBLE);
        mForwardRate = 1;
        mRewindRate = 1;
        isRewindOrForward = false;
        mVideoView.resume();
      }
      /** end add by yangjie 2011/11/17 */
      if (mVideoList != null) {
        /** start add by yangjie 2011/11/17 */
        CommonData.isResume = false;
        mPlayButton.setBackgroundResource(R.drawable.pause);
        /** end add by yangjie 2011/11/17 */
        VideoFile videoModel = mVideoList.getNextVideo();
        if (null == videoModel) {
          finish();
        } else {
          mCurrentVideoPath = videoModel.path;
          mVideoView.setVideoPath(mCurrentVideoPath);
        }
      }
      break;
    case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
    case KeyEvent.KEYCODE_PAGE_UP:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        return true;
      }
      mCurrentTrackID = 0;
      mCurrentSubtitleID = 0;
      mCurrentChannelPostion = 0;
      /** start add by yangjie 2011/11/17 */
      if (isRewindOrForward) {
        mForwardRewindText.setVisibility(View.INVISIBLE);
        mForwardRate = 1;
        mRewindRate = 1;
        isRewindOrForward = false;
        mVideoView.resume();
      }
      /** end add by yangjie 2011/11/17 */
      if (mVideoList != null) {
        /** start add by yangjie 2011/11/17 */
        CommonData.isResume = false;
        mPlayButton.setBackgroundResource(R.drawable.pause);
        /** end add by yangjie 2011/11/17 */
        VideoFile videoModel = mVideoList.getPreviousVideo();
        if (null == videoModel) {
          finish();
        } else {
          mCurrentVideoPath = videoModel.path;
          mVideoView.setVideoPath(mCurrentVideoPath);
        }
      }
      break;
    case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        return true;
      }
      if (isRewindOrForward) {
        mForwardRewindText.setVisibility(View.INVISIBLE);
        mForwardRate = 1;
        mRewindRate = 1;
        isRewindOrForward = false;
        mVideoView.resume();
        mPlayButton.setBackgroundResource(R.drawable.pause);
      } else {
        play_pause();
      }
      return true;
    case KeyEvent.KEYCODE_ENTER:
    case KeyEvent.KEYCODE_DPAD_CENTER:
      if (mVideoMenuLayout != null && mVideoMenuLayout.isShown()) {
        if (mVideoPlayModeMenuLayout != null
            && mVideoPlayModeMenuLayout.isFocused()) {
          if (mPlayMode == VideoConstant.PLAY_ALL_CYCLE) {
            mPlayMode = VideoConstant.PLAY_ONE_SINGLE_TIME;
            SharedPreferencesHelper.setValue(mContext,
                SharedPreferencesHelper.XML_VIDEO_SETTINGS,
                SharedPreferencesHelper.KEY_VIDEO_PLAY_MODE,
                VideoConstant.PLAY_ONE_SINGLE_TIME);
          } else {
            mPlayMode = VideoConstant.PLAY_ALL_CYCLE;
            SharedPreferencesHelper.setValue(mContext,
                SharedPreferencesHelper.XML_VIDEO_SETTINGS,
                SharedPreferencesHelper.KEY_VIDEO_PLAY_MODE,
                VideoConstant.PLAY_ALL_CYCLE);
          }
        } else if (mFullScreenContrlMenuLayout != null
            && mFullScreenContrlMenuLayout.isFocused()) {
          if (isFullScreen) {
            setVideoScale(VideoConstant.SCREEN_DEFAULT);
            if (mMediaControllerLayout != null) {
              mMediaControllerLayout.setVisibility(View.VISIBLE);
              if (!mMediaControllerHandler
                  .hasMessages(VideoConstant.SHOW_MEDIA_CONTROLLER)) {
                mMediaControllerHandler
                    .sendEmptyMessage(VideoConstant.SHOW_MEDIA_CONTROLLER);
              }
              mAutoHideHandler.removeCallbacks(mAutoHideRunnable);
              isAutoHideThreadStart = false;
            }
            if (mFullScreenContrlTitle != null)
              mFullScreenContrlTitle.setText(R.string.full_screen);
          } else {
            setVideoScale(VideoConstant.SCREEN_FULL);
            startAutoHideThread();
            if (mFullScreenContrlTitle != null)
              mFullScreenContrlTitle.setText(R.string.exit_full_screen);
          }
          isFullScreen = !isFullScreen;
        } else if (mSwitchSubtitleMenuLayout != null
            && mSwitchSubtitleMenuLayout.isFocused()) {
          switchSubtitle();
        } else if (mSwitchTrackMenuLayout != null
            && mSwitchTrackMenuLayout.isFocused()) {
          switchAudioTrack();
        } else if (mSwitchChannelMenuLayout != null
            && mSwitchChannelMenuLayout.isFocused()) {
          switchChannel();
        }
        mVideoMenuLayout.setVisibility(View.GONE);
        return true;
      }

      if (isRewindOrForward) {
        mForwardRewindText.setVisibility(View.INVISIBLE);
        mForwardRate = 1;
        mRewindRate = 1;
        isRewindOrForward = false;
        mVideoView.resume();
        mPlayButton.setBackgroundResource(R.drawable.pause);
      } else {
        play_pause();
      }
      break;
    default:
      break;
    }
    return super.onKeyDown(keyCode, event);
  }

  private Handler mMediaControllerHandler = new Handler() {
    public void handleMessage(Message msg) {
      String methodName = "mMediaControllerHandler.handleMessage";
      if (ISDEBUG) logd(methodName, "what = " + msg.what);
      switch (msg.what) {
      case VideoConstant.SHOW_MEDIA_CONTROLLER:
        msg = obtainMessage(VideoConstant.SHOW_MEDIA_CONTROLLER);
        if (!isSeeking && mVideoView.isPlaying()) {
          float position = mVideoView.getCurrentPosition();
          mVideoSeekBar.setProgress((int) (position / mDuration * mSeekBarMax));
          mCurrentTimeText.setText(Converter
              .convertToHoursMinutesSeconds((int) position));
        }
        sendMessageDelayed(msg, 1000);
        break;
      case VideoConstant.HIDE_MEDIA_CONTROLLER:
        removeMessages(VideoConstant.SHOW_MEDIA_CONTROLLER);
        removeMessages(VideoConstant.HIDE_MEDIA_CONTROLLER);
        break;
      default:
        break;
      }
    }
  };

  class AutoHideRunnable implements Runnable {
    public void run() {
      long endTime = System.currentTimeMillis();
      long distance = endTime - mPreHideStartTime;
      if (distance >= VideoConstant.CTRLBAR_HIDE_TIME) {
        if (isFullScreen) {
          if (mVideoView.isPlaying()) {
            ctrlbarDismiss();
          }
        } else {
          mAutoHideHandler.removeCallbacks(this);
          isAutoHideThreadStart = false;
        }
      }
      mAutoHideHandler.postDelayed(this, 1000);
    }
  }

  private void startAutoHideThread() {
    if (!isAutoHideThreadStart) {
      mPreHideStartTime = System.currentTimeMillis();
      mMediaControllerHandler
          .sendEmptyMessage(VideoConstant.SHOW_MEDIA_CONTROLLER);
      mAutoHideHandler.post(mAutoHideRunnable);
      isAutoHideThreadStart = true;
    } else {
      mPreHideStartTime = System.currentTimeMillis();
    }
  }

  private void ctrlbarDismiss() {
    mMediaControllerLayout.setVisibility(View.GONE);
    mAutoHideHandler.removeCallbacks(mAutoHideRunnable);
    isAutoHideThreadStart = false;
    mMediaControllerHandler
        .sendEmptyMessage(VideoConstant.HIDE_MEDIA_CONTROLLER);
  }

  private void switchChannel() {
    if (null == mAudioChannels) {
      mAudioChannels = new String[] { getString(R.string.stereo), // 0
          getString(R.string.mono), // 1
          getString(R.string.left_channel_double), // 2
          getString(R.string.right_channel_double), // 3
          getString(R.string.right_channel_single), // 5
          getString(R.string.left_channel_single) // 6
      };
    }
    createCustomDialog(this, "channel", getString(R.string.switch_channel),
        mCurrentChannelPostion, mAudioChannels, mChannelOnItemClickListener,
        450, 250);

    // ---------------test------------------
    // Parcel parcelReader;
    // parcelReader = getInfo(7);
    // parcelReader.readInt(); // read return value
    // int AUDIO_CHANNEL_MODE = parcelReader.readInt();
    // Log.d(TAG, "------current audio channel mode: " +
    // AUDIO_CHANNEL_MODE);
  }

  private void switchAudioTrack() {
    Parcel parcelReader;
    /* Don't use the following functions for performance */
    // parcelReader = getInfo(VideoConstant.CMD_GET_AUDIO_TRACK_PID);
    // parcelReader.readInt(); // read return value
    // int currentTrackID = parcelReader.readInt(); // read audio track pid
    // Log.d(TAG, "current selectedTrack: " + currentTrackID);

    parcelReader = getInfo(VideoConstant.CMD_GET_AUDIO_INFO);
    parcelReader.readInt(); // read return value
    int trackCount = parcelReader.readInt(); // read audio track pid count
    String[] tracks = { getResources().getString(R.string.noTrack) };
    if (trackCount > 0) {
      tracks = new String[trackCount];
      for (int i = 0; i < trackCount; i++) {
        tracks[i] = parcelReader.readString(); // read language
        parcelReader.readInt(); // read audio format
        parcelReader.readInt(); // read audio sample rate
        parcelReader.readInt(); // read audio channels count
        if (tracks[i].equals(""))
          tracks[i] = getResources().getString(R.string.soundTrack) + " "
              + (i + 1);
      }
    }

    /* System dialog start ------ */
    // Dialog switchTrackDialog = new AlertDialog.Builder(this)
    // .setTitle(getString(R.string.switch_audio_track))
    // .setIcon(android.R.drawable.ic_dialog_info)
    // .setSingleChoiceItems(tracks, currentTrackID, mTrackOnClickListener)
    // .setNegativeButton(getString(R.string.cancel), null).show();
    // WindowManager.LayoutParams params = switchTrackDialog.getWindow()
    // .getAttributes();
    // params.width = 300;
    // params.x = 400;
    // switchTrackDialog.getWindow().setAttributes(params);
    /* System dialog end ------ */

    createCustomDialog(this, "track", getString(R.string.switch_audio_track),
        mCurrentTrackID, tracks, mTrackOnItemClickListener, 450, 250);
  }

  private void switchSubtitle() {
    Parcel parcelReader;
    /* Don't use the following functions for performance */
    // Parcel parcelReader = getInfo(VideoConstant.CMD_GET_SUB_ID);
    // parcelReader.readInt();
    // mCurrentSubtitleID = parcelReader.readInt();

    int subtitleCount = 0;
    parcelReader = getInfo(VideoConstant.CMD_GET_SUB_INFO);
    if (0 == parcelReader.readInt()) { // Read return value.
      subtitleCount = parcelReader.readInt(); // Read subtitle number.
    }

    String[] subtitleNames = { getResources().getString(R.string.nosubTitle) };
    if (0 != subtitleCount) {
      subtitleNames = new String[subtitleCount];
      for (int i = 0; i < subtitleCount; i++) {
        parcelReader.readInt(); // Read subtitle ID.
        parcelReader.readInt(); // Read 'ExtSubTitle'.
        String subtitleName = parcelReader.readString(); // Read
                                                         // 'aszSubTitleName'.
        if (TextUtils.isEmpty(subtitleName)) {
          subtitleNames[i] = getResources().getString(R.string.subtitle) + " "
              + (i + 1);
        } else {
          subtitleNames[i] = subtitleName;
        }
      }

      /* System dialog start ------ */
      // Dialog switchSubtitleDialog = new AlertDialog.Builder(this)
      // .setTitle(getString(R.string.switch_subtitle))
      // .setIcon(android.R.drawable.ic_dialog_info)
      // .setSingleChoiceItems(subtitleNames, mCurrentSubtitleID,
      // mSubtitleOnClickListener)
      // .setNegativeButton(getString(R.string.cancel), null).show();
      // WindowManager.LayoutParams params =
      // switchSubtitleDialog.getWindow()
      // .getAttributes();
      // params.width = 300;
      // params.x = 400;
      // switchSubtitleDialog.getWindow().setAttributes(params);
      /* System dialog end ------ */

      createCustomDialog(this, "subtitle", getString(R.string.switch_subtitle),
          mCurrentSubtitleID, subtitleNames, mSubtitleOnItemClickListener, 450,
          250);
    } else {
      /* System dialog start ------ */
      // Dialog noSubtitleDialog = new AlertDialog.Builder(this)
      // .setTitle(getString(R.string.nosubTitle))
      // .setIcon(android.R.drawable.ic_dialog_info)
      // .setPositiveButton(getString(R.string.confirm), null).show();
      // WindowManager.LayoutParams params = noSubtitleDialog.getWindow()
      // .getAttributes();
      // params.width = 300;
      // params.x = 400;
      // noSubtitleDialog.getWindow().setAttributes(params);
      /* System dialog end ------ */

      createCustomDialog(this, "subtitle", getString(R.string.nosubTitle),
          mCurrentSubtitleID, null, null, 500, 200);
    }
  }

  private AdapterView.OnItemClickListener mChannelOnItemClickListener = new AdapterView.OnItemClickListener() {
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
        long id) {
      int channelID = 0;
      switch (position) {
      case 0:
        channelID = 0;
        break;
      case 1:
        channelID = 1;
        break;
      case 2:
        channelID = 2;
        break;
      case 3:
        channelID = 3;
        break;
      case 4:
        channelID = 5;
        break;
      case 5:
        channelID = 6;
        break;

      default:
        break;
      }
      setNewFounction(VideoConstant.CMD_SET_CHANNEL, channelID);
      mCurrentChannelPostion = position;
    }
  };

  private AdapterView.OnItemClickListener mTrackOnItemClickListener = new AdapterView.OnItemClickListener() {
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
        long id) {
      setNewFounction(VideoConstant.CMD_SET_AUDIO_TRACK_PID, position);
      mCurrentTrackID = position;
    }
  };

  private AdapterView.OnItemClickListener mSubtitleOnItemClickListener = new AdapterView.OnItemClickListener() {
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
        long id) {
      setNewFounction(VideoConstant.CMD_SET_SUB_ID, position);
      mCurrentSubtitleID = position;
    }
  };

  private void createCustomDialog(Context context, String flag,
      String dialogTitle, int selectedItem, String[] items,
      final AdapterView.OnItemClickListener onItemClickListener, int x,
      int width) {
    if (null == context) {
      return;
    }

    String buttonString = "";
    if (null == items || 0 == items.length) {
      buttonString = context.getString(R.string.confirm);
      if ("track".equals(flag)) {
        dialogTitle = context.getString(R.string.noTrack);
      } else if ("subtitle".equals(flag)) {
        dialogTitle = context.getString(R.string.nosubTitle);
      }
    } else {
      buttonString = context.getString(R.string.cancel);
    }

    final Dialog dialog = new Dialog(context, R.style.dialog_style_1);

    LayoutInflater inflater = ((Activity) context).getLayoutInflater();
    View mDialogContentView = inflater.inflate(R.layout.custom_dialog_layout_1,
        null);
    TextView mDialogTitle = (TextView) mDialogContentView
        .findViewById(R.id.dialog_1_head_title);
    mDialogTitle.setText(dialogTitle);
    ListView menuItemListView = (ListView) mDialogContentView
        .findViewById(R.id.dialog_1_body_listview);
    Button dialogButton = (Button) mDialogContentView
        .findViewById(R.id.dialog_1_foot_button);
    dialogButton.setText(buttonString);
    dialogButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        dialog.dismiss();
      }
    });

    menuItemListView.setFocusableInTouchMode(true);
    menuItemListView.setAdapter(new MenuItemListAdapter(this, items));
    menuItemListView.setSelection(selectedItem);
    menuItemListView.setSelector(R.drawable.bg_dialog_2);
    menuItemListView
        .setOnItemClickListener(new AdapterView.OnItemClickListener() {
          @Override
          public void onItemClick(AdapterView<?> parent, View view,
              int position, long id) {
            if (null != onItemClickListener) {
              onItemClickListener.onItemClick(parent, view, position, id);
            }
            dialog.dismiss();
          }
        });

    dialog.setContentView(mDialogContentView);
    WindowManager.LayoutParams layoutParams = dialog.getWindow()
        .getAttributes();
    layoutParams.width = width;
    layoutParams.x = x;
    dialog.getWindow().setAttributes(layoutParams);
    dialog.show();
  }

  private void getScreenSize() {
    // modifty
    Display display = getWindowManager().getDefaultDisplay();
    // mScreenHeight = displayInfo.getNaturalHeight();
    // mScreenWidth = displayInfo.getNaturalWidth();
    Point size = new Point();
    display.getSize(size);
    mScreenHeight = size.y;
    mScreenWidth = size.x;
  }

  private void setVideoScale(int flag) {
    String methodName = "setVideoScale";
    if (ISDEBUG) logd(methodName, "flag = " + flag);
    switch (flag) {
    case VideoConstant.SCREEN_FULL: /* Full screen */
      mVideoView.setVideoScale(mScreenWidth, mScreenHeight);
      getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
      break;
    case VideoConstant.SCREEN_DEFAULT: /* Normal screen */
      /* Video width and height */
      int videoWidth = mVideoView.getVideoWidth();
      int videoHeight = mVideoView.getVideoHeight();
      if (ISDEBUG) logd(methodName, "videoWidth = " + videoWidth + ", videoHeight = "
          + videoHeight);

      /* Screen width and height, 43 is height of statusbar */
      int mHeight = mScreenHeight - 2
          * mMediaControllerLayout.getLayoutParams().height - 43;
      int mWidth = mScreenWidth * mHeight / mScreenHeight;
      if (ISDEBUG) logd(methodName, "mWidth:" + mWidth + " mHeight:" + mHeight);
      if (videoWidth > 0 && videoHeight > 0) {
        if (videoWidth * mHeight > mWidth * videoHeight) {
          mHeight = mWidth * videoHeight / videoWidth;

        } else if (videoWidth * mHeight <= mWidth * videoHeight) {
          mWidth = videoWidth * mHeight / videoHeight;
        }
      }
      mVideoView.setVideoScale(mWidth, mHeight);

      /* Leave status bar */
      getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
      break;
    }
  }

  private void play_pause() {
    String methodName = "play_pause";
    if (CommonData.isResume) {
      try {
        Thread.sleep(1000);
        CommonData.isResume = false;
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    if (mVideoView.isPlaying()) {
      mPlayButton.setBackgroundResource(R.drawable.play);
      mVideoView.pause();
      DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PAUSED;
      // add liumeidong 20131107 begin
      if (mAirPlayService != null) {
        String returnJson = "{" + "\"sender\": \"xbmc\","
            + "\"status\": \"OnPause\"}";
        if (ISDEBUG) logd(methodName, "returnJson = " + returnJson);
        try {
          mAirPlayService.announceToClients(returnJson);
        } catch (RemoteException e) {
          loge(methodName, e);
        }
      }
      // add liumeidong 20131107 end
    } else {
      if (this.isStop) {
        /*
         * Afer stop, mVideoView turns to be null,so it is necessory to get
         * mVideoView again
         */
        if (ISDEBUG) logd(methodName, "isStop =" + this.isStop);
        mVideoView = (VideoView) findViewById(R.id.videoView);
        mVideoView.setVideoPath(mCurrentVideoPath);
        mPlayButton.setBackgroundResource(R.drawable.pause);
        this.isStop = false;
      } else {

        mPlayButton.setBackgroundResource(R.drawable.pause);
        start();
        if (mAirPlayService != null) {
          String returnJson = "{" + "\"sender\": \"xbmc\","
              + "\"status\": \"OnPlay\"}";
          if (ISDEBUG) logd(methodName, "returnJson = " + returnJson);
          try {
            mAirPlayService.announceToClients(returnJson);
          } catch (RemoteException e) {
            loge(methodName, e);
          }
        }
      }
    }
  }

  private void start() {
    if (mVideoView != null) {
      mVideoView.start();
      mForwardRate = 1;
      mRewindRate = 1;
      mForwardRewindText.setVisibility(View.INVISIBLE);
      // for test
      mVideoView.setSubDisplay(mSubHolder);
    }
    mVideoView = (VideoView) findViewById(R.id.videoView);
    DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PLAYING;
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    return super.onCreateOptionsMenu(menu);
  }

  private void seek(boolean isForward) {
    int currentPlayTime = mVideoView.getCurrentPosition();
    float targetPlayTime;
    int targetSeekbarPostion;
    if (isForward) {
      targetPlayTime = currentPlayTime + mSeekStep;
    } else {
      targetPlayTime = currentPlayTime - mSeekStep;
    }
    targetSeekbarPostion = (int) (targetPlayTime / mDuration * mSeekBarMax);
    if (targetPlayTime > mDuration) {
      targetPlayTime = mDuration;
      targetSeekbarPostion = mSeekBarMax;
    } else if (targetPlayTime <= 0) {
      targetPlayTime = 0;
      targetSeekbarPostion = 0;
    }
    isSeeking = true;
    mVideoSeekBar.setProgress(targetSeekbarPostion);
    mCurrentTimeText.setText(Converter
        .convertToHoursMinutesSeconds((int) targetPlayTime));
    mVideoView.seekTo((int) targetPlayTime);
    if (isFullScreen) {
      startAutoHideThread();
    }
    isSeeking = false;
  }

  private void seekTo(float targetPlayTime) {
    String mehtodName = "seekTo";
    int targetSeekbarPostion;
    targetSeekbarPostion = (int) (targetPlayTime / mDuration * mSeekBarMax);
    if (targetPlayTime > mDuration) {
      targetPlayTime = mDuration;
      targetSeekbarPostion = mSeekBarMax;
    } else if (targetPlayTime <= 0) {
      targetPlayTime = 0;
      targetSeekbarPostion = 0;
    }
    isSeeking = true;
    mVideoSeekBar.setProgress(targetSeekbarPostion);
    mCurrentTimeText.setText(Converter
        .convertToHoursMinutesSeconds((int) targetPlayTime));
    mVideoView.seekTo((int) targetPlayTime);
    if (ISDEBUG) logd(mehtodName, "videoView seek to: " + targetPlayTime);
    if (isFullScreen) {
      startAutoHideThread();
    }
    isSeeking = false;
  }

  private void preUpdateProgressBar() {
    if (null != mVideoView && null != mVideoSeekBar && null != mCurrentTimeText) {
      float position = mVideoView.getCurrentPosition();
      mVideoSeekBar.setProgress((int) (position / mDuration * mSeekBarMax));
      mCurrentTimeText.setText(Converter
          .convertToHoursMinutesSeconds((int) position));
    }
  }

  private void fastForwardPlay() {
    if (mForwardRate < 32) {
      if (!(mVideoView.isPlaying())) {
        mVideoView.start();
      }
      mRewindRate = 1;
      mForwardRate *= 2;
      setNewFounction(VideoConstant.FORWARD_RATE, mForwardRate);
      if (!mForwardRewindText.isShown()) {
        mForwardRewindText.setVisibility(View.VISIBLE);
      }
      mForwardRewindText.setText(mForwardRate + " X");
      mPlayButton.setBackgroundResource(R.drawable.pause);
      isRewindOrForward = true;
    }
  }

  private void rewindPlay() {
    if (mRewindRate < 32) {
      if (!(mVideoView.isPlaying())) {
        mVideoView.start();
      }
      mForwardRate = 1;
      mRewindRate *= 2;
      setNewFounction(VideoConstant.REWIND_RATE, mRewindRate);
      if (!mForwardRewindText.isShown()) {
        mForwardRewindText.setVisibility(View.VISIBLE);
      }
      mForwardRewindText.setText("-" + mRewindRate + " X");
      mPlayButton.setBackgroundResource(R.drawable.pause);
      isRewindOrForward = true;
    }
  }

  private Parcel getInfo(int flag) {
    Parcel requestParcel = Parcel.obtain();
    requestParcel.writeInterfaceToken("android.media.IMediaPlayer");
    requestParcel.writeInt(flag);
    Parcel replayParcel = Parcel.obtain();
    mVideoView.invoke(requestParcel, replayParcel);
    replayParcel.setDataPosition(0);
    return replayParcel;
  }

  private void setNewFounction(int flag, int rate) {
    if (ISDEBUG) logd(TAG, "flag =" + flag + ", rate = " + rate);
    Parcel requestParcel = Parcel.obtain();
    requestParcel.writeInterfaceToken("android.media.IMediaPlayer");
    requestParcel.writeInt(flag);
    requestParcel.writeInt(rate);
    Parcel replayParcel = Parcel.obtain();
    mVideoView.invoke(requestParcel, replayParcel);
  }

  private class DMRMessageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
      String methodName = "onReceive";
      String action = intent.getAction();
      if (ISDEBUG) logd(methodName, action);
      if (PublicConstants.DMRIntent.ACTION_DMR_EVENT.equals(action)) {
        int cmd = intent
            .getIntExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD, -1);
        if (ISDEBUG) logd(methodName, "cmd:" + cmd);
        switch (cmd) {
        case PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY:
          if (isRewindOrForward) {
            if (ISDEBUG) logd(methodName, "the VideoView is isRewindOrForward()");
            mForwardRewindText.setVisibility(View.INVISIBLE);
            mForwardRate = 1;
            mRewindRate = 1;
            isRewindOrForward = false;
            mVideoView.resume();
            mPlayButton.setBackgroundResource(R.drawable.pause);
          } else {
            if (VideoPlayer.this.isStop) {
              /*
               * Afer stop, mVideoView turns to be null,so it is necessory to
               * get mVideoView again
               */
              if (ISDEBUG) logd(methodName, "the VideoView is stop()");
              mVideoView = (VideoView) findViewById(R.id.videoView);
              mVideoView.setVideoPath(mCurrentVideoPath);
              mPlayButton.setBackgroundResource(R.drawable.pause);
              VideoPlayer.this.isStop = false;
            } else {
              if (ISDEBUG) logd(methodName, "the VideoView is stop() mVideoView = "
                  + mVideoView);
              mPlayButton.setBackgroundResource(R.drawable.pause);
              start();

            }
          }
          if (isFullScreen) {
            if (mVideoMenuLayout == null || !mVideoMenuLayout.isShown()) {
              mMediaControllerLayout.setVisibility(View.VISIBLE);
              startAutoHideThread();
            }
          }
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE:
          if (ISDEBUG) logd(methodName,"recevice the commond = "
                  + PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE);
          if (isRewindOrForward) {
            mForwardRewindText.setVisibility(View.INVISIBLE);
            mForwardRate = 1;
            mRewindRate = 1;
            isRewindOrForward = false;
            mVideoView.resume();
            mPlayButton.setBackgroundResource(R.drawable.play);
            mVideoView.pause();
            DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PAUSED;
          } else {
            mPlayButton.setBackgroundResource(R.drawable.play);
            mVideoView.pause();
            DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PAUSED;
          }
          if (isFullScreen) {
            if (mVideoMenuLayout == null || !mVideoMenuLayout.isShown()) {
              mMediaControllerLayout.setVisibility(View.VISIBLE);
              startAutoHideThread();
            }
          }
          // add liumeidong 20131107 begin
          if (mAirPlayService != null) {
            String returnJson = "{" + "\"sender\": \"xbmc\","
                + "\"status\": \"OnPause\" }";
            if (ISDEBUG) logd(methodName, "returnJson = " + returnJson);
            try {
              mAirPlayService.announceToClients(returnJson);
            } catch (RemoteException e) {
              loge(methodName, e);
            }
          }
          // add liumeidong 20131107 end
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK:
          int intCurrentPosition = 0;
          if (null != mVideoView) {
            intCurrentPosition = mVideoView.getCurrentPosition();
          }
          int seekValue = intent.getIntExtra(
              PublicConstants.DMRIntent.KEY_SEEK_VALUE, intCurrentPosition);
          if (ISDEBUG) logd(methodName, "seek to: " + seekValue);
          seekTo(seekValue);
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP:
          if (ISDEBUG) logd(methodName,
              "recevice the PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP "
                  + PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP);
          DMRSharingInformation.resetDMRShareData();
          finish();
          if (mAirPlayService != null) {
            String returnJson = "{" + "\"sender\": \"xbmc\","
                + "\"status\": \"OnStop\"}";
            if (ISDEBUG) logd(methodName, "returnJson = " + returnJson); 
            try {
              mAirPlayService.announceToClients(returnJson);
            } catch (RemoteException e) {
              e.printStackTrace();
            }
          }
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME:
          if (null != mVideoView) {
            mVideoView.setLRVolume(DMRSharingInformation.sLeftVolume,
                DMRSharingInformation.sRightVolume);
          }
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME:
           if (ISDEBUG) logd(methodName, "resume the Video player");
          play_pause();
          break;

        default:
          break;
        }
      }
    }
  }

  private boolean stopUpdateDMRInformation = true;

  private void startUpdateDMRInformation() {
    if (stopUpdateDMRInformation) {
      stopUpdateDMRInformation = false;
      new Thread(new UpdateDMRInformationTimerRunnable()).start();
    }
  }

  private void stopUpdateDMRInformation() {
    stopUpdateDMRInformation = true;
  }

  private class UpdateDMRInformationTimerRunnable implements Runnable {
    @Override
    public void run() {
      try {
        Thread.currentThread();
        while (!stopUpdateDMRInformation) {
          Thread.sleep(500);
          if (stopUpdateDMRInformation) {
            return;
          }
          if (null != mVideoView) {
            mCurrentPlayPostion = String.valueOf(mVideoView
                .getCurrentPosition() / 1000);
          }
        }
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

  }

  private String mCurrentPlayPostion;

  public static class MediaPlayerInfoListener implements
      MediaPlayer.OnInfoListener {
    @Override
    public boolean onInfo(MediaPlayer arg0, int what, int arg1) {
      if (ISDEBUG) logd("onInfo", "what=" + what + ", arg1 = " + arg1);
      return false;
    }
  }

  private static void logd(String methodName, String tag) {
    Log.d(TAG, "[" + methodName + ", " + tag + "]");
  }

  private static void loge(String methodName, Exception ex) {
    Log.e(TAG, methodName + ", exception = ", ex);
  }
}
