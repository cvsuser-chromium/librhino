package com.hybroad.music;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.lang.ref.SoftReference;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Random;
import java.util.TreeMap;

import android.R.color;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.media.MediaMetadataRetriever;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.hybroad.dlna.DMPMessageTransit;
import com.hybroad.dlna.DMRSharingInformation;
import com.hybroad.dlna.OnCurrentDMSChanged;
import com.hybroad.media.DevicesList;
import com.hybroad.media.R;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.FileEncodeParser;
import com.hybroad.util.IOUtitls;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.PublicConstants.DialogReslut;
import com.hybroad.util.PublicConstants.IntentStarter;
import com.hybroad.util.SharedPreferencesHelper;
import com.hybroad.view.AutoMarqueeTextView;
import com.hybroad.view.LyricView;
import com.hybroad.view.MyListView;
import com.hybroad.view.MySeekBar;

public class MusicPlayer extends Activity {
  private final String TAG = "----MusicPlayer----";

  private RelativeLayout musicControllor;
  private ImageButton playPauseButton;
  private ImageView playModeImageView;
  private ImageView albumImageView;
  private TextView timeCurrentView;
  // private TextView timeCurrentExplicitView;
  private TextView playModeTextView;
  private TextView timeTotalView;
  private LyricView mLyricView;
  private AutoMarqueeTextView audioTitleView;
  private AutoMarqueeTextView singerView;
  private MySeekBar seekBar;
  private MyListView musicListView;

  private View musicMenuLayout;
  private View randomPlayMenuLayout;
  private View singleTrackMenuLayout;
  private View orderPlayMenuLayout;
  private View favoriteOperateMenuLayout;
  private ImageView favoriteOperateImageView;
  private TextView favoriteOperateTextView;

  private int intMusicDuration = 0;
  private int intCurrentPosition = 0;
  private int intSeekStep = 0;
  private int firstEntertargetPosition;
  private int targetMusicPlayPosition;
  private String audioFilePath;
  private int musicPlayMode;
  private long startSwitchMusicTime = 0;
  private long startHideSeekBarTime = 0;

  private boolean isPlaying = false;
  private boolean userTouchSeekBar = false;
  private boolean needAvoidOncePlay = true;
  private boolean isSwitchMusic = false;
  private boolean isSeeking = false;
  private boolean switchMusicHandlerHasPosted = false;
  private boolean hideSeekBarHandlerHasPosted = false;
  private boolean externalLrcExists = false;

  private MusicUIReceiver mMusicUIReceiver;
  private MusicUIReceiver mMusicMediaReceiver;
  private Intent CMDIntent;
  private ArrayList<LrcUnit> lrcSentenceList;
  private Handler switchMusicHandler = null;
  private SwitchMusicTimerRunnable mSwitchMusicTimerRunnable = null;
  private Handler hideSeekBarTimerHandler = null;
  private HideSeekBarRunnable mHideSeekBarRunnable = null;
  private HashMap<String, SoftReference<Bitmap>> imageCacheHashMap;
  private boolean isParseLrcThreadLocked = false;
  private Thread mParseLrcThread = null;
  private long mParseLrcThreadCount = 0;
  private StorageManager mStorageManager = null;
  private int mCurrentChannelPostion = 0;
  private String[] mAudioChannels = null;

  private String mPlayType = MusicConstant.PLAYLIST_TYPE_NORMAL;
  private int mPlayIntentType = IntentStarter.NORMAL_START;
  private String mDMRMusicPath;
  private int mDMRTargetSeekPostion = 0;
  private int mDMRDialogResult = DialogReslut.NOTHING;
  private DMPMessageTransit mDMPMessageTransit;
  private DMRMessageReceiver mDMRMessageReceiver;

  /** Called when the activity is first created. */
  @Override
  public void onCreate(Bundle savedInstanceState) {
    Log.i(TAG, "----onCreate()----");
    super.onCreate(savedInstanceState);

    mDMRMessageReceiver = new DMRMessageReceiver();
    IntentFilter mDMRFilter = new IntentFilter();
    mDMRFilter.addAction(PublicConstants.DMRIntent.ACTION_DMR_EVENT);
    mDMRFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_PREPARED);
    registerReceiver(mDMRMessageReceiver, mDMRFilter);

    Intent intent = getIntent();
    String starter = intent
        .getStringExtra(PublicConstants.IntentStarter.NORMAL_START_MUSIC_KEY);
    if (!TextUtils.isEmpty(starter)
        && starter
            .equals(PublicConstants.IntentStarter.NORMAL_START_MUSIC_NAME)) {
      mPlayIntentType = IntentStarter.NORMAL_START;
      setContentView(R.layout.music_player_main);
      firstEntertargetPosition = getIntent().getIntExtra(
          PublicConstants.SELECTED_ITEM_POSITION, 0);
      targetMusicPlayPosition = firstEntertargetPosition;
      mPlayType = getIntent().getStringExtra(MusicConstant.PLAYLIST_TYPE);
      startMusicPlayService(targetMusicPlayPosition);
    } else {
      mPlayIntentType = IntentStarter.DMR_START;
      mDMRMusicPath = intent.getStringExtra(PublicConstants.DMRIntent.KEY_URL);
      if (TextUtils.isEmpty(mDMRMusicPath)) {
        finish();
      }
      mDMRTargetSeekPostion = intent.getIntExtra(
          PublicConstants.DMRIntent.KEY_SEEK_VALUE, 0);
      String originalTitle = intent
          .getStringExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE);

      // popDMRRequestDialogOnCreate();
      MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
      MusicFile music = new MusicFile();
      music.filePath = mDMRMusicPath;

      if (TextUtils.isEmpty(originalTitle)) {
        int indexStart = mDMRMusicPath.lastIndexOf('/') + 1;
        int indexEnd = mDMRMusicPath.lastIndexOf('.');
        if (indexStart >= 0 && indexStart < indexEnd
            && indexEnd < mDMRMusicPath.length()) {
          music.title = mDMRMusicPath.substring(indexStart, indexEnd);
        } else {
          music.title = mDMRMusicPath;
        }
      } else {
        music.title = originalTitle;
      }

      music.setTypeMusic();
      MusicPlayList.sCurrentMusicList.add(music);

      setContentView(R.layout.music_player_main);
      startMusicPlayService(0);
      initView();
    }

    CMDIntent = new Intent(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
  }

  @Override
  protected void onNewIntent(Intent intent) {
    Log.d(TAG, "----onNewIntent----");
    super.onNewIntent(intent);
    setIntent(intent);

    mDMRMusicPath = intent.getStringExtra(PublicConstants.DMRIntent.KEY_URL);
    if (TextUtils.isEmpty(mDMRMusicPath)) {
      return; // do nothing
    }
    mDMRTargetSeekPostion = intent.getIntExtra(
        PublicConstants.DMRIntent.KEY_SEEK_VALUE, 0);
    String originalTitle = intent
        .getStringExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE);

    // popDMRRequestDialogOnNewIntent();
    mPlayIntentType = IntentStarter.DMR_START;
    MusicPlayList.sCurrentMusicList = null; // don't use clear(), or backup is
                                            // also clear
    MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
    MusicFile music = new MusicFile();
    music.filePath = mDMRMusicPath;

    if (TextUtils.isEmpty(originalTitle)) {
      int indexStart = mDMRMusicPath.lastIndexOf('/') + 1;
      int indexEnd = mDMRMusicPath.lastIndexOf('.');
      if (indexStart >= 0 && indexStart < indexEnd
          && indexEnd < mDMRMusicPath.length()) {
        music.title = mDMRMusicPath.substring(indexStart, indexEnd);
      } else {
        music.title = mDMRMusicPath;
      }
    } else {
      music.title = originalTitle;
    }

    music.setTypeMusic();
    MusicPlayList.sCurrentMusicList.add(music);

    setContentView(R.layout.music_player_main);
    startMusicPlayService(0);
    initView();
  }

  @Override
  protected void onRestart() {
    Log.d(TAG, "----onRestart----");
    super.onRestart();
  }

  @Override
  protected void onStart() {
    Log.d(TAG, "----onStart----");
    super.onStart();

    if (IntentStarter.NORMAL_START == mPlayIntentType) {
      initView();
    }

    registerReceiver();

    if (null != DevicesList.s_currentDevice
        && DevicesList.s_currentDevice.isLocalDevice()) {
      if (mStorageManager == null) {
        mStorageManager = (StorageManager) MusicPlayer.this
            .getSystemService(STORAGE_SERVICE);
      }
//      mStorageManager.registerListener(mStorageEventListener);
    } else if (null != DevicesList.s_currentDevice
        && DevicesList.s_currentDevice.isDMSDevice()) {
      if (null == mDMPMessageTransit) {
        mDMPMessageTransit = new DMPMessageTransit(new OnCurrentDMSChanged() {
          @Override
          public void onDMSDeviceDown() {
            CustomWidget.toastDeviceDown(MusicPlayer.this);
            finish();
          }
        });
      }
      IntentFilter filter = new IntentFilter(
          PublicConstants.DMPIntent.ACTION_DMS_DEVICE_CHANGE);
      registerReceiver(mDMPMessageTransit, filter);
    }
  }

  @Override
  protected void onResume() {
    Log.d(TAG, "----onResume----");
    super.onResume();
  }

  @Override
  protected void onPause() {
    Log.d(TAG, "----onPause----");
    super.onPause();
  }

  @Override
  protected void onStop() {
    Log.d(TAG, "----onStop----");
    super.onStop();

    stopService(new Intent(PublicConstants.MusicCommon.MUSIC_PLAY_SERVICE));

    if (null != mMusicUIReceiver) {
      unregisterReceiver(mMusicUIReceiver);
    }
    if (null != mMusicMediaReceiver) {
      unregisterReceiver(mMusicMediaReceiver);
    }
    if (null != mStorageManager) {
//      mStorageManager.unregisterListener(mStorageEventListener);
    }
    if (null != mDMPMessageTransit) {
      unregisterReceiver(mDMPMessageTransit);
    }
  }

  @Override
  protected void onDestroy() {
    Log.d(TAG, "----onDestroy----");
    super.onDestroy();
    if (MusicConstant.PLAYLIST_TYPE_FAVORITE.equals(mPlayType))
      MusicPlayList.sCurrentMusicList = MusicPlayList.sCurrentMusicListBackup;

    if (IntentStarter.DMR_START == mPlayIntentType) {
      MusicPlayList.sCurrentMusicList = MusicPlayList.sCurrentMusicListBackup;
      DMRSharingInformation.resetDMRShareData();
    }
    if (null != mDMRMessageReceiver) {
      unregisterReceiver(mDMRMessageReceiver);
    }
  }

  private void popDMRRequestDialogOnCreate() {
    Dialog playDMRDialog = new AlertDialog.Builder(this)
        .setIcon(android.R.drawable.ic_dialog_info)
        .setTitle(getString(R.string.dmr_play_request))
        .setMessage(
            getString(R.string.dmr_play_request_content) + mDMRMusicPath)
        .setPositiveButton(getString(R.string.confirm),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
                MusicFile music = new MusicFile();
                music.filePath = mDMRMusicPath;
                int indexStart = mDMRMusicPath.lastIndexOf('/') + 1;
                int indexEnd = mDMRMusicPath.lastIndexOf('.');
                if (indexStart >= 0 && indexStart < indexEnd
                    && indexEnd < mDMRMusicPath.length()) {
                  music.title = mDMRMusicPath.substring(indexStart, indexEnd);
                } else {
                  music.title = mDMRMusicPath;
                }
                music.setTypeMusic();
                MusicPlayList.sCurrentMusicList.add(music);

                mDMRDialogResult = DialogReslut.CONFIRM;
                dialog.dismiss();

                setContentView(R.layout.music_player_main);
                startMusicPlayService(0);
                initView();
              }
            })
        .setNegativeButton(getString(R.string.cancel),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                dialog.dismiss();
                if (mDMRDialogResult != DialogReslut.CONFIRM) // used to
                                                              // continuous
                                                              // repeated
                                                              // requests
                                                              // pending case
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
            getString(R.string.dmr_play_request_content) + mDMRMusicPath)
        .setPositiveButton(getString(R.string.confirm),
            new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialog, int arg1) {
                mPlayIntentType = IntentStarter.DMR_START;
                MusicPlayList.sCurrentMusicList = null; // don't use clear(), or
                                                        // backup is also clear
                MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
                MusicFile music = new MusicFile();
                music.filePath = mDMRMusicPath;
                int indexStart = mDMRMusicPath.lastIndexOf('/') + 1;
                int indexEnd = mDMRMusicPath.lastIndexOf('.');
                if (indexStart >= 0 && indexStart < indexEnd
                    && indexEnd < mDMRMusicPath.length()) {
                  music.title = mDMRMusicPath.substring(indexStart, indexEnd);
                } else {
                  music.title = mDMRMusicPath;
                }
                music.setTypeMusic();
                MusicPlayList.sCurrentMusicList.add(music);

                mDMRDialogResult = DialogReslut.CONFIRM; // used to continuous
                                                         // repeated requests
                                                         // pending case
                dialog.dismiss();

                setContentView(R.layout.music_player_main);
                startMusicPlayService(0);
                initView();
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

  private void initView() {
    initLayoutView();

    // Fix for the first time into the player without drawing album.
    albumImageView.setImageResource(R.drawable.music_album_default);

    initAudioDataAndPlayView(targetMusicPlayPosition);
    if (MusicPlayList.sCurrentMusicList != null && targetMusicPlayPosition >= 0
        && targetMusicPlayPosition < MusicPlayList.sCurrentMusicList.size()) {
      if (audioTitleView != null) {
        audioTitleView.setText(MusicPlayList.sCurrentMusicList
            .get(targetMusicPlayPosition).title);
      }
      if (singerView != null) {
        if (null != MusicPlayList.sCurrentMusicList
            .get(targetMusicPlayPosition).artist)
          singerView.setText(MusicPlayList.sCurrentMusicList
              .get(targetMusicPlayPosition).artist);
        else
          new AsyncRetrieveMetaData().execute(targetMusicPlayPosition);
      }
    }

    // autoHideMusicControllor();
  }

  private void registerReceiver() {
    mMusicUIReceiver = new MusicUIReceiver();
    IntentFilter mMusicUIFilter = new IntentFilter();
    mMusicUIFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_DURATION);
    mMusicUIFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_FLUSH_TIME);
    mMusicUIFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_FINISH);
    mMusicUIFilter
        .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR);
    registerReceiver(mMusicUIReceiver, mMusicUIFilter);

    mMusicMediaReceiver = new MusicUIReceiver();
    IntentFilter mMusicMediaFilter = new IntentFilter();
    mMusicMediaFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
    mMusicMediaFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
    mMusicMediaFilter.addAction(Intent.ACTION_MEDIA_EJECT);
    mMusicMediaFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
    mMusicMediaFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
    mMusicMediaFilter.addDataScheme("file"); // important!!!
    registerReceiver(mMusicMediaReceiver, mMusicMediaFilter);
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

  private void initLayoutView() {
    musicControllor = (RelativeLayout) this.findViewById(R.id.musicControllor);
    seekBar = (MySeekBar) this.findViewById(R.id.musicSeekBar);
    timeCurrentView = (TextView) this.findViewById(R.id.timeCurrent);
    playModeTextView = (TextView) this.findViewById(R.id.playMode_text);
    timeTotalView = (TextView) this.findViewById(R.id.timeTotal);
    mLyricView = (LyricView) this.findViewById(R.id.lrc_content);
    mLyricView.setSentenceList(lrcSentenceList);
    mLyricView.setNotCurrentPaintColor(Color.WHITE);
    mLyricView.setCurrentPaintColor(Color.GREEN);
    mLyricView.setLrcTextSize(23);
    mLyricView.setLyricBackgroundColor(Color.TRANSPARENT);
    mLyricView.setTextGap(40);
    audioTitleView = (AutoMarqueeTextView) this.findViewById(R.id.musicTitle);
    singerView = (AutoMarqueeTextView) this.findViewById(R.id.singer);
    playModeImageView = (ImageView) this.findViewById(R.id.playMode_image);
    albumImageView = (ImageView) this.findViewById(R.id.albumImage);
    playPauseButton = (ImageButton) this.findViewById(R.id.play_pause);

    musicMenuLayout = this.findViewById(R.id.music_menu_layout_content);
    randomPlayMenuLayout = this.findViewById(R.id.random_play_menu_layout);
    singleTrackMenuLayout = this.findViewById(R.id.single_track_menu_layout);
    orderPlayMenuLayout = this.findViewById(R.id.order_play_menu_layout);
    favoriteOperateMenuLayout = this
        .findViewById(R.id.favorite_operate_menu_layout);
    favoriteOperateImageView = (ImageView) this
        .findViewById(R.id.favorite_operate_menu_icon);
    favoriteOperateTextView = (TextView) this
        .findViewById(R.id.favorite_operate_menu_title);

    switchMusicHandler = new Handler();
    mSwitchMusicTimerRunnable = new SwitchMusicTimerRunnable();
    hideSeekBarTimerHandler = new Handler();
    mHideSeekBarRunnable = new HideSeekBarRunnable();

    createListView();

    musicPlayMode = SharedPreferencesHelper.getValue(this,
        SharedPreferencesHelper.XML_MUSIC_SETTINGS,
        SharedPreferencesHelper.KEY_MUSIC_PLAY_MODE,
        MusicConstant.PLAY_MODE_ORDER);
    setPlayModeView(musicPlayMode);

    playPauseButton.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View v) {
        ImageButton button = (ImageButton) v;
        if (isPlaying) {
          pause();
          button.setBackgroundResource(R.drawable.play);
        } else {
          play();
          button.setBackgroundResource(R.drawable.pause);
          // autoHideMusicControllor();
        }
      }
    });

    seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
      @Override
      public void onStopTrackingTouch(SeekBar seekBar) {
        seekTo(seekBar.getProgress());
        userTouchSeekBar = false;
      }

      @Override
      public void onStartTrackingTouch(SeekBar seekBar) {
        userTouchSeekBar = true;

      }

      @Override
      public void onProgressChanged(SeekBar seekBar, int progress,
          boolean fromUser) {
        timeCurrentView.setText(getTimeFormatForMusicTime(progress));
        // timeCurrentExplicitView.setText(getTimeFormatForMusicTime(progress));
      }
    });

  }

  public String getTimeFormatForMusicTime(int time) {
    String timeFormat = "";
    int flag = time / 60000;
    if (flag < 10) {
      timeFormat = "0" + time / 60000;
    } else {
      timeFormat = "" + time / 60000;
    }
    flag = time % 60000 / 1000;
    if (flag < 10) {
      timeFormat += ":0" + flag;
    } else {
      timeFormat += ":" + flag;
    }
    return timeFormat;
  }

  class MusicUIReceiver extends BroadcastReceiver {

    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      if (action.equals(PublicConstants.MusicCommon.INTENT_ACTION_DURATION)) {
        intMusicDuration = intent.getExtras().getInt(
            PublicConstants.MusicCommon.INTENT_KEY_DURATION);
        DMRSharingInformation.sDuration = String
            .valueOf(intMusicDuration / 1000);
        if (seekBar != null) {
          seekBar.setMax(intMusicDuration);
        }
        if (timeTotalView != null) {
          timeTotalView.setText(getTimeFormatForMusicTime(intMusicDuration));
        }
        intSeekStep = intMusicDuration / 10;
      } else if (action
          .equals(PublicConstants.MusicCommon.INTENT_ACTION_FLUSH_TIME)) {
        intCurrentPosition = intent.getExtras().getInt("intCurrentPosition");

        if (DMRSharingInformation.sDuration == "0") {
          DMRSharingInformation.sCurrentPlayPostion = "0";
        } else {
          DMRSharingInformation.sCurrentPlayPostion = String
              .valueOf(intCurrentPosition / 1000);
        }

        // updateExplicitCurrentTime(intCurrentPosition);
        updateTimeSeekBar(intCurrentPosition);
        if (externalLrcExists) {
          updateLrcView();
        }
      } else if (action
          .equals(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_FINISH)
          || action
              .equals(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR)) {
        mCurrentChannelPostion = 0;
        if (IntentStarter.DMR_START == mPlayIntentType) {
          DMRSharingInformation.resetDMRShareData();
          if (action
              .equals(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR)) {
            finish();
          }
          return;
        }
        switch (musicPlayMode) {
        case MusicConstant.PLAY_MODE_ORDER:
          if (MusicPlayList.sCurrentMusicList != null) {
            if (targetMusicPlayPosition < MusicPlayList.sCurrentMusicList
                .size() - 1) {
              targetMusicPlayPosition++;
            } else if (targetMusicPlayPosition == MusicPlayList.sCurrentMusicList
                .size() - 1) {
              targetMusicPlayPosition = 0;
            }
            if (musicListView != null) {
              musicListView.requestFocus();
              musicListView.setSelection(targetMusicPlayPosition);
            }
          }
          break;
        case MusicConstant.PLAY_MODE_RANDOM:
          if (MusicPlayList.sCurrentMusicList != null && musicListView != null) {
            // targetMusicPlayPosition =
            // getShufflerNumber(sCurrentMusicList.size(),
            // targetMusicPlayPosition);
            targetMusicPlayPosition = getShufflerNumber(
                MusicPlayList.sCurrentMusicList.size(), targetMusicPlayPosition);
            musicListView.requestFocus();
            musicListView.setSelection(targetMusicPlayPosition);
          }
          break;
        case MusicConstant.PLAY_MODE_SINGLE:
          startOneNewMusic(targetMusicPlayPosition);
          break;

        default:
          break;
        }
      } else if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {

      } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {

      } else if (action.equals(Intent.ACTION_MEDIA_SCANNER_FINISHED)) {

      } else if (action.equals(Intent.ACTION_MEDIA_EJECT)) {

      }
    } // end onReceive

  } // end MusicUIReceiver

  private void startOneNewMusic(int targetPosition) {
    // targetMusicPlayPosition = targetPosition; // update new position
    startMusicPlayService(targetPosition);
    initAudioDataAndPlayView(targetPosition);
  }

  private void startMusicPlayService(int targetPosition) {
    if (MusicPlayList.sCurrentMusicList == null || targetPosition < 0
        || targetPosition >= MusicPlayList.sCurrentMusicList.size()) {
      Log.d(TAG, "star a new music----failure!");
      return;
    }
    audioFilePath = MusicPlayList.sCurrentMusicList.get(targetPosition).filePath;
    Intent serviceIntent = new Intent(
        PublicConstants.MusicCommon.MUSIC_PLAY_SERVICE);
    serviceIntent.putExtra(
        PublicConstants.MusicCommon.INTENT_KEY_MUSIC_FILE_PATH, audioFilePath);
    serviceIntent.putExtra(PublicConstants.MusicCommon.INTENT_KEY_SEEK_VALUE,
        mDMRTargetSeekPostion);
    startService(serviceIntent);
    isPlaying = true;
    Log.d(TAG, "star a new music----ok!");
  }

  private void initAudioDataAndPlayView(int targetPosition) {
    if (MusicPlayList.sCurrentMusicList == null || targetPosition < 0
        || targetPosition >= MusicPlayList.sCurrentMusicList.size()) {
      return;
    }
    intCurrentPosition = 0;
    intMusicDuration = MusicPlayList.sCurrentMusicList.get(targetPosition).duration;
    intSeekStep = intMusicDuration / 10;

    // if (timeCurrentExplicitView != null) {
    // timeCurrentExplicitView.setText(getTimeFormatForMusicTime(0));
    // }
    if (seekBar != null) {
      seekBar.setMax(intMusicDuration);
    }
    if (timeTotalView != null) {
      timeTotalView.setText(getTimeFormatForMusicTime(intMusicDuration));
    }
    if (playPauseButton != null) {
      playPauseButton.setBackgroundResource(R.drawable.pause);
    }
    updateTimeSeekBar(0);

    if (MusicPlayList.sCurrentMusicList.get(targetPosition).embededImageExists == 1) {
      Bitmap bitmap = getArtworkFromCache(MusicPlayList.sCurrentMusicList
          .get(targetPosition).filePath);
      if (null != bitmap) {
        BitmapDrawable bmpDraw = new BitmapDrawable(bitmap);
        if (null != bmpDraw) {
          albumImageView.setImageDrawable(bmpDraw);
        }
      } else {
        new AsyncUpdateAlbumTask().execute(targetPosition);
      }
    }

    cleanLyricView();
    readLrc(getLrcPath(audioFilePath));
  }

  private Bitmap getArtwork(String filePath) {
    Bitmap bitmap = null;
    bitmap = getArtworkFromCache(filePath);
    if (bitmap != null) {
      return bitmap;
    }
    bitmap = MusicUtils.getArtworkFromFile(filePath);
    if (bitmap != null) {
      if (imageCacheHashMap == null) {
        imageCacheHashMap = new HashMap<String, SoftReference<Bitmap>>();
      }
      imageCacheHashMap.put(filePath, new SoftReference<Bitmap>(bitmap));
    }
    return bitmap;
  }

  private Bitmap getArtworkFromCache(String filePath) {
    Bitmap bitmap = null;
    if (imageCacheHashMap == null) {
      imageCacheHashMap = new HashMap<String, SoftReference<Bitmap>>();
      return null;
    }
    if (imageCacheHashMap.containsKey(filePath)) {
      SoftReference<Bitmap> softReference = imageCacheHashMap.get(filePath);
      bitmap = softReference.get();
      if (bitmap == null) {
        imageCacheHashMap.remove(filePath);
      }
    }
    return bitmap;
  }

  private void play() {
    if (null == CMDIntent) {
      CMDIntent = new Intent(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
    }
    CMDIntent.putExtra("playerCommand", MusicConstant.CMD_PLAY);
    sendBroadcast(CMDIntent);
    isPlaying = true;
    DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PLAYING;
  }

  private void pause() {
    if (null == CMDIntent) {
      CMDIntent = new Intent(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
    }
    CMDIntent.putExtra("playerCommand", MusicConstant.CMD_PAUSE);
    sendBroadcast(CMDIntent);
    isPlaying = false;
    DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_PAUSED;
  }

  private void seekTo(int msec) {
    if (null == CMDIntent) {
      CMDIntent = new Intent(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
    }
    CMDIntent.putExtra("playerCommand", MusicConstant.CMD_SEEK);
    CMDIntent.putExtra("seekValue", msec);
    sendBroadcast(CMDIntent);
  }

  private void stop() {
    if (null == CMDIntent) {
      CMDIntent = new Intent(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
    }
    CMDIntent.putExtra("playerCommand", MusicConstant.CMD_STOP);
    sendBroadcast(CMDIntent);
    isPlaying = false;
  }

  private void setPlayMode(int mode) {
    musicPlayMode = mode;
    SharedPreferencesHelper.setValue(this,
        SharedPreferencesHelper.XML_MUSIC_SETTINGS,
        SharedPreferencesHelper.KEY_MUSIC_PLAY_MODE, musicPlayMode);
    setPlayModeView(mode);
  }

  private void setPlayModeView(int mode) {
    if (playModeImageView != null) {
      switch (mode) {
      case MusicConstant.PLAY_MODE_ORDER:
        playModeImageView.setBackgroundResource(R.drawable.order_play);
        playModeTextView.setText(R.string.orderPlay);
        break;
      case MusicConstant.PLAY_MODE_RANDOM:
        playModeImageView.setBackgroundResource(R.drawable.random_play);
        playModeTextView.setText(R.string.randomPlay);
        break;
      case MusicConstant.PLAY_MODE_SINGLE:
        playModeImageView.setBackgroundResource(R.drawable.single_track);
        playModeTextView.setText(R.string.singleTrack);
        break;

      default:
        break;
      }
    }
  }

  private void updateTimeSeekBar(int msec) {
    if (musicControllor != null && musicControllor.isShown()
        && !userTouchSeekBar && !isSwitchMusic) {
      if (timeCurrentView != null) {
        timeCurrentView.setText(getTimeFormatForMusicTime(msec));
      }
      if (seekBar != null) {
        seekBar.setProgress(msec);
      }
    }
  }

  private void updateExplicitCurrentTime(int msec) {
    // if (!isSeeking && !userTouchSeekBar && !isSwitchMusic &&
    // timeCurrentExplicitView != null) {
    // timeCurrentExplicitView.setText(getTimeFormatForMusicTime(msec));
    // }
  }

  private void createListView() {
    if (MusicPlayList.sCurrentMusicList == null) {
      return;
    }
    musicListView = (MyListView) this.findViewById(R.id.musicListView);
    MusicListItemAdapter listItemAdapter = new MusicListItemAdapter(this,
        MusicPlayList.sCurrentMusicList);
    musicListView.setAdapter(listItemAdapter);
    musicListView.setSelector(color.transparent);
    musicListView.setOnItemSelectedListener(new OnItemSelectedListener() {

      @Override
      public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
          long arg3) {
        if (arg2 == firstEntertargetPosition) {
          if (needAvoidOncePlay) {
            needAvoidOncePlay = false;
          } else {
            isSwitchMusic = true;
            musicControllor.setVisibility(View.INVISIBLE);
            // timeCurrentExplicitView.setVisibility(View.INVISIBLE);
            pause();
            fastUpdateMusicAlbumInfo(arg2);
            targetMusicPlayPosition = arg2;
            switchMusicAfterSomeTime();
          }
        } else {
          isSwitchMusic = true;
          musicControllor.setVisibility(View.INVISIBLE);
          // timeCurrentExplicitView.setVisibility(View.INVISIBLE);
          pause();
          fastUpdateMusicAlbumInfo(arg2);
          targetMusicPlayPosition = arg2;
          switchMusicAfterSomeTime();
        }
      }

      @Override
      public void onNothingSelected(AdapterView<?> arg0) {
        musicListView.setSelection(targetMusicPlayPosition);
      }
    });

    View tempView;
    if ((tempView = getCurrentFocus()) != null) {
      tempView.clearFocus();
    }
    musicListView.requestFocus();
    musicListView.setSelection(targetMusicPlayPosition);

  }

  class SwitchMusicTimerRunnable implements Runnable {
    public void run() {
      long endTime = System.currentTimeMillis();
      long distance = endTime - startSwitchMusicTime;
      if (switchMusicHandler == null) {
        switchMusicHandler = new Handler();
      }
      if (distance >= 600) {
        startOneNewMusic(targetMusicPlayPosition);
        // if (null != timeCurrentExplicitView) {
        // timeCurrentExplicitView.setVisibility(View.VISIBLE);
        // }
        if (null != musicControllor) {
          musicControllor.setVisibility(View.VISIBLE);
        }
        isSwitchMusic = false;
        switchMusicHandler.removeCallbacks(mSwitchMusicTimerRunnable);
        switchMusicHandlerHasPosted = false;
        Log.d(TAG, "----removeCallbacks(mSwitchMusicTimerRunnable)----");
      } else {
        switchMusicHandler.postDelayed(this, 200);
      }
    }
  }

  private void switchMusicAfterSomeTime() {
    if (switchMusicHandler == null) {
      switchMusicHandler = new Handler();
    }
    if (switchMusicHandlerHasPosted) {
      switchMusicHandler.removeCallbacks(mSwitchMusicTimerRunnable);
      switchMusicHandlerHasPosted = false;
    }
    startSwitchMusicTime = System.currentTimeMillis();
    switchMusicHandler.post(mSwitchMusicTimerRunnable);
    switchMusicHandlerHasPosted = true;
  }

  class HideSeekBarRunnable implements Runnable {
    @Override
    public void run() {
      long endTime = System.currentTimeMillis();
      long distance = endTime - startHideSeekBarTime;
      if (hideSeekBarTimerHandler == null) {
        hideSeekBarTimerHandler = new Handler();
      }
      if (distance >= 5000) {
        if (isPlaying) {
          musicControllor.setVisibility(View.INVISIBLE);
        }
        hideSeekBarTimerHandler.removeCallbacks(mHideSeekBarRunnable);
        hideSeekBarHandlerHasPosted = false;
        Log.d(TAG, "----removeCallbacks(mHideSeekBarRunnable)----");
      } else {
        hideSeekBarTimerHandler.postDelayed(this, 500);
      }
    }
  }

  private void autoHideMusicControllor() {
    if (hideSeekBarTimerHandler == null) {
      hideSeekBarTimerHandler = new Handler();
    }
    if (hideSeekBarHandlerHasPosted) {
      hideSeekBarTimerHandler.removeCallbacks(mHideSeekBarRunnable);
      hideSeekBarHandlerHasPosted = false;
    }
    startHideSeekBarTime = System.currentTimeMillis();
    hideSeekBarTimerHandler.post(mHideSeekBarRunnable);
    hideSeekBarHandlerHasPosted = true;
  }

  private void fastUpdateMusicAlbumInfo(int position) {
    if (MusicPlayList.sCurrentMusicList == null || position < 0
        || position >= MusicPlayList.sCurrentMusicList.size()) {
      return;
    }
    if (audioTitleView != null) {
      audioTitleView
          .setText(MusicPlayList.sCurrentMusicList.get(position).title);
    }
    if (singerView != null) {
      if (null != MusicPlayList.sCurrentMusicList.get(position).artist)
        singerView
            .setText(MusicPlayList.sCurrentMusicList.get(position).artist);
      else
        new AsyncRetrieveMetaData().execute(position);
    }
    if (albumImageView != null) {
      if (MusicPlayList.sCurrentMusicList.get(position).embededImageExists == 1) {
        Bitmap bitmap = getArtworkFromCache(MusicPlayList.sCurrentMusicList
            .get(position).filePath);
        if (bitmap != null) {
          BitmapDrawable bmpDraw = new BitmapDrawable(bitmap);
          albumImageView.setImageDrawable(bmpDraw);
        } else {
          albumImageView.setImageResource(R.drawable.music_album_default);
        }
      } else {
        albumImageView.setImageResource(R.drawable.music_album_default);
      }
    }
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    switch (keyCode) {
    case KeyEvent.KEYCODE_DPAD_LEFT:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        if (favoriteOperateMenuLayout != null
            && favoriteOperateMenuLayout.isFocused()
            && orderPlayMenuLayout != null) {
          orderPlayMenuLayout.requestFocus();
        } else if (orderPlayMenuLayout != null
            && orderPlayMenuLayout.isFocused() && singleTrackMenuLayout != null) {
          singleTrackMenuLayout.requestFocus();
        } else if (singleTrackMenuLayout != null
            && singleTrackMenuLayout.isFocused()
            && randomPlayMenuLayout != null) {
          randomPlayMenuLayout.requestFocus();
        }
        return true;
      }
      // if (musicControllor != null) {
      // musicControllor.setVisibility(View.VISIBLE);
      // }
      seekTo(intCurrentPosition - intSeekStep);
      // if (isPlaying) {
      // autoHideMusicControllor();
      // }
      return true;

    case KeyEvent.KEYCODE_MEDIA_REWIND:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        return true;
      }
      // if (musicControllor != null) {
      // musicControllor.setVisibility(View.VISIBLE);
      // }
      seekTo(intCurrentPosition - intSeekStep);
      // if (isPlaying) {
      // autoHideMusicControllor();
      // }
      return true;

    case KeyEvent.KEYCODE_DPAD_RIGHT:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        if (randomPlayMenuLayout != null && randomPlayMenuLayout.isFocused()
            && singleTrackMenuLayout != null) {
          singleTrackMenuLayout.requestFocus();
        } else if (singleTrackMenuLayout != null
            && singleTrackMenuLayout.isFocused() && orderPlayMenuLayout != null) {
          orderPlayMenuLayout.requestFocus();
        } else if (orderPlayMenuLayout != null
            && orderPlayMenuLayout.isFocused()
            && favoriteOperateMenuLayout != null) {
          favoriteOperateMenuLayout.requestFocus();
        }
        return true;
      }
      // if (musicControllor != null) {
      // musicControllor.setVisibility(View.VISIBLE);
      // }
      seekTo(intCurrentPosition + intSeekStep);
      // if (isPlaying) {
      // autoHideMusicControllor();
      // }
      return true;

    case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
      if (musicMenuLayout.isShown()) {
        return true;
      }
      // if (musicControllor != null) {
      // musicControllor.setVisibility(View.VISIBLE);
      // }
      seekTo(intCurrentPosition + intSeekStep);
      // if (isPlaying) {
      // autoHideMusicControllor();
      // }
      return true;

    case KeyEvent.KEYCODE_DPAD_CENTER:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        if (randomPlayMenuLayout != null && randomPlayMenuLayout.isFocused()) {
          musicMenuLayout.setVisibility(View.INVISIBLE);
          setPlayMode(MusicConstant.PLAY_MODE_RANDOM);
        } else if (singleTrackMenuLayout != null
            && singleTrackMenuLayout.isFocused()) {
          musicMenuLayout.setVisibility(View.INVISIBLE);
          setPlayMode(MusicConstant.PLAY_MODE_SINGLE);
        } else if (orderPlayMenuLayout != null
            && orderPlayMenuLayout.isFocused()) {
          musicMenuLayout.setVisibility(View.INVISIBLE);
          setPlayMode(MusicConstant.PLAY_MODE_ORDER);
        } else if (favoriteOperateMenuLayout != null
            && favoriteOperateMenuLayout.isFocused()) {
          musicMenuLayout.setVisibility(View.INVISIBLE);
          if (MusicConstant.PLAYLIST_TYPE_FAVORITE.equals(mPlayType)) { // remove
                                                                        // from
                                                                        // favorites
            removeFromFavorite();
          } else { // enter favorites
            enterMyFavorite();
          }
        }
        if (musicListView != null) {
          musicListView.requestFocus();
          musicListView.setSelection(targetMusicPlayPosition);
        }
        return true;
      }
      if (isPlaying) {
        pause();
        if (playPauseButton != null) {
          playPauseButton.setBackgroundResource(R.drawable.play);
        }
        // if (musicControllor != null) {
        // musicControllor.setVisibility(View.VISIBLE);
        // }
      } else {
        play();
        if (playPauseButton != null) {
          playPauseButton.setBackgroundResource(R.drawable.pause);
        }
        // autoHideMusicControllor();
      }
      return true;

    case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        return true;
      }
      if (isPlaying) {
        pause();
        if (playPauseButton != null) {
          playPauseButton.setBackgroundResource(R.drawable.play);
        }
        // if (musicControllor != null) {
        // musicControllor.setVisibility(View.VISIBLE);
        // }
      } else {
        play();
        if (playPauseButton != null) {
          playPauseButton.setBackgroundResource(R.drawable.pause);
        }
        // autoHideMusicControllor();
      }
      return true;

    case KeyEvent.KEYCODE_DPAD_DOWN:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        return true;
      }
      if (musicListView != null
          && (musicListView.getSelectedItemPosition() == musicListView
              .getCount() - 1)) {
        musicListView.setSelection(0);
        return true;
      }
      break;

    case KeyEvent.KEYCODE_DPAD_UP:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        return true;
      }
      if (musicListView != null
          && (musicListView.getSelectedItemPosition() == 0)) {
        musicListView.setSelection(musicListView.getCount() - 1);
        return true;
      }
      break;

    case KeyEvent.KEYCODE_MENU:
      if (musicMenuLayout != null) {
        if (musicMenuLayout.isShown()) {
          musicMenuLayout.setVisibility(View.INVISIBLE);
          if (musicListView != null) {
            musicListView.requestFocus();
            musicListView.setSelection(targetMusicPlayPosition);
          }
        } else {
          if (MusicConstant.PLAYLIST_TYPE_FAVORITE.equals(mPlayType)) {
            if (null != favoriteOperateImageView)
              favoriteOperateImageView.setImageResource(R.drawable.remove);
            if (null != favoriteOperateTextView)
              favoriteOperateTextView.setText(R.string.remove_from_favorite);
          } else {
            if (null != favoriteOperateImageView)
              favoriteOperateImageView
                  .setImageResource(R.drawable.enter_favorite);
            if (null != favoriteOperateTextView)
              favoriteOperateTextView.setText(R.string.enter_favorite);
          }
          musicMenuLayout.setVisibility(View.VISIBLE);
          if (null != randomPlayMenuLayout)
            randomPlayMenuLayout.requestFocus();
        }
      }
      return true;

    case PublicConstants.IR_KEY_AUDIO_CHANNEL:
      switchChannel();
      return true;

    case KeyEvent.KEYCODE_BACK:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        musicMenuLayout.setVisibility(View.INVISIBLE);
        if (musicListView != null) {
          musicListView.requestFocus();
          musicListView.setSelection(targetMusicPlayPosition);
        }
        return true;
      } else {
        return super.onKeyDown(keyCode, event);
      }

    case KeyEvent.KEYCODE_MEDIA_STOP:
      if (musicMenuLayout != null && musicMenuLayout.isShown()) {
        return true;
      }
      stop();
      DMRSharingInformation.resetDMRShareData();
      finish();

    default:
      break;
    }

    return super.onKeyDown(keyCode, event);
  }

  private void removeFromFavorite() {
    // 1. save the path of target, and remove from
    // 'MusicPlayList.sCurrentMusicList'
    String targetRemovePath = "";
    if (null != MusicPlayList.sCurrentMusicList) {
      MusicFile audio = MusicPlayList.sCurrentMusicList
          .get(targetMusicPlayPosition);
      if (null == audio)
        return;
      targetRemovePath = audio.filePath;
      MusicPlayList.sCurrentMusicList.remove(targetMusicPlayPosition);
    }

    // 2. refresh view
    if (null == MusicPlayList.sCurrentMusicList
        || MusicPlayList.sCurrentMusicList.isEmpty()) {
      stop();
      IOUtitls.deleteFileOrFolder(targetRemovePath);
      finish();
    }
    startMusicPlayService(targetMusicPlayPosition);
    MusicListItemAdapter listItemAdapter = new MusicListItemAdapter(this,
        MusicPlayList.sCurrentMusicList);
    if (musicListView != null)
      musicListView.setAdapter(listItemAdapter);

    if (albumImageView != null)
      albumImageView.setImageResource(R.drawable.music_album_default);

    if (MusicPlayList.sCurrentMusicList != null && targetMusicPlayPosition >= 0
        && targetMusicPlayPosition < MusicPlayList.sCurrentMusicList.size()) {
      if (audioTitleView != null) {
        audioTitleView.setText(MusicPlayList.sCurrentMusicList
            .get(targetMusicPlayPosition).title);
      }
      if (singerView != null) {
        if (null != MusicPlayList.sCurrentMusicList
            .get(targetMusicPlayPosition).artist)
          singerView.setText(MusicPlayList.sCurrentMusicList
              .get(targetMusicPlayPosition).artist);
        else
          new AsyncRetrieveMetaData().execute(targetMusicPlayPosition);
      }
    }

    initAudioDataAndPlayView(targetMusicPlayPosition);

    // 3. delete file from nand flash
    IOUtitls.deleteFileOrFolder(targetRemovePath);
  }

  private void enterMyFavorite() {
    File directory = new File(PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH);
    File[] files = null;
    if (directory.exists() && directory.isDirectory())
      files = directory.listFiles();
    if (null == files || 0 == files.length) {
      Dialog favoriteDialog = new AlertDialog.Builder(this)
          .setTitle(getString(R.string.favorite_is_empty))
          .setIcon(android.R.drawable.ic_dialog_info)
          .setPositiveButton(getString(R.string.confirm), null).show();
      WindowManager.LayoutParams params = favoriteDialog.getWindow()
          .getAttributes();
      params.width = 300;
      favoriteDialog.getWindow().setAttributes(params);
    } else { // really enter favorites
      mPlayType = MusicConstant.PLAYLIST_TYPE_FAVORITE;
      MusicPlayList.sCurrentMusicList = null;
      MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
      for (File file : files) {
        if (file.isFile()) {
          MusicFile audio = new MusicFile();
          audio.filePath = file.getAbsolutePath();
          audio.title = file.getName();
          audio.title = audio.title.substring(0, audio.title.lastIndexOf('.'));
          audio.setTypeMusic();
          MusicPlayList.sCurrentMusicList.add(audio);
        }
      }
      firstEntertargetPosition = 0;
      targetMusicPlayPosition = firstEntertargetPosition;
      startMusicPlayService(targetMusicPlayPosition);

      MusicListItemAdapter listItemAdapter = new MusicListItemAdapter(this,
          MusicPlayList.sCurrentMusicList);
      if (musicListView != null)
        musicListView.setAdapter(listItemAdapter);

      if (albumImageView != null)
        albumImageView.setImageResource(R.drawable.music_album_default);

      if (MusicPlayList.sCurrentMusicList != null
          && targetMusicPlayPosition >= 0
          && targetMusicPlayPosition < MusicPlayList.sCurrentMusicList.size()) {
        if (audioTitleView != null) {
          audioTitleView.setText(MusicPlayList.sCurrentMusicList
              .get(targetMusicPlayPosition).title);
        }
        if (singerView != null) {
          if (null != MusicPlayList.sCurrentMusicList
              .get(targetMusicPlayPosition).artist)
            singerView.setText(MusicPlayList.sCurrentMusicList
                .get(targetMusicPlayPosition).artist);
          else
            new AsyncRetrieveMetaData().execute(targetMusicPlayPosition);
        }
      }

      initAudioDataAndPlayView(targetMusicPlayPosition);
    }
  }

  private int getShufflerNumber(int interval, int previous) {
    Random mRandom = new Random();
    int ret;
    do {
      ret = mRandom.nextInt(interval);
      // Returns a pseudo-random uniformly distributed int in the half-open
      // range [0, interval).
    } while (ret == previous && interval > 1);
    return ret;
  }

  private String getLrcPath(String songPath) {
    String lrcPath = null;
    int lastPoint = -1;
    if (songPath != null) {
      lastPoint = songPath.lastIndexOf('.');
      if (lastPoint >= 0 && lastPoint < songPath.length()) {
        lrcPath = songPath.substring(0, lastPoint) + ".lrc";
        Log.d(TAG, "lrcPath:" + lrcPath);
      }
    }
    return lrcPath;
  }

  private void readLrc(String path) {
    stopCurrentParseLrcThread();
    if (path == null || mLyricView == null) {
      return;
    }
    if (lrcSentenceList == null) {
      lrcSentenceList = new ArrayList<LrcUnit>(); // lrc is empty at this time
    }
    // cleanLyricView();
    startNewParseLrcThread(path);
  }

  private void cleanLyricView() {
    if (null != lrcSentenceList && null != mLyricView) {
      lrcSentenceList.clear();
      mLyricView.setSentenceList(lrcSentenceList); // emmpty lrc
      mLyricView.updateIndex(getCurrentSentenceIndex()); // index = -1
      mLyricView.invalidate(); // clear old view
    }
  }

  private void updateLrcView() {
    if (!isSeeking && !userTouchSeekBar && !isSwitchMusic && mLyricView != null) {
      mLyricView.updateIndex(getCurrentSentenceIndex());
      mLyricView.invalidate();
    }
  }

  private int getCurrentSentenceIndex() {
    int index = -1;
    if (lrcSentenceList != null && lrcSentenceList.size() > 0) {
      for (int i = 0; i < lrcSentenceList.size(); i++) {
        LrcUnit mLrcUnit = lrcSentenceList.get(i);
        if (mLrcUnit != null) {
          if ((intCurrentPosition + 10 > mLrcUnit.getBeginTime())
              && (intCurrentPosition + 10 < mLrcUnit.getBeginTime()
                  + mLrcUnit.getLineTime())) {
            index = i;
            break;
          }
        }
      }
    }
    return index;
  }

  private Handler setLyricHandler = new Handler() {
    public void handleMessage(Message msg) {
      if (mLyricView != null && lrcSentenceList != null) {
        mLyricView.setSentenceList(lrcSentenceList);
      }
    }
  };

  private void startNewParseLrcThread(String path) {
    stopCurrentParseLrcThread();
    mParseLrcThread = new Thread(new ParseLrcRunnable(mParseLrcThreadCount,
        path));
    mParseLrcThread.start();
  }

  private void stopCurrentParseLrcThread() {
    mParseLrcThreadCount++;
    mParseLrcThread = null;
  }

  class ParseLrcRunnable implements Runnable {
    private long mThreadID = 0;
    private String mFilePath = null;

    public ParseLrcRunnable(long count, String path) {
      ParseLrcRunnable.this.mThreadID = count;
      ParseLrcRunnable.this.mFilePath = path;
    }

    @Override
    public void run() {
      Log.d(TAG, "==>mParseLrcThread----start: mCount=" + mThreadID
          + ", count=" + mParseLrcThreadCount);
      if (this.mThreadID != mParseLrcThreadCount) {
        Log.d(TAG, "==>mParseLrcThread----early termination: mCount="
            + mThreadID + ", count=" + mParseLrcThreadCount);
        return;
      }

      while (isParseLrcThreadLocked) {
        try {
          Log.d(TAG, "mParseLrcThread:" + this.mThreadID
              + " is locked, waitting......");
          Thread.currentThread();
          Thread.sleep(1000);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
        if (this.mThreadID != mParseLrcThreadCount) {
          Log.d(TAG, "==>mParseLrcThread----early termination: mCount="
              + mThreadID + ", count=" + mParseLrcThreadCount);
          return;
        }
      }
      isParseLrcThreadLocked = true;

      // work area
      {
        if (TextUtils.isEmpty(mFilePath)) {
          externalLrcExists = false;
          isParseLrcThreadLocked = false;
          return;
        }

        TreeMap<Integer, LrcUnit> lrcTemp = new TreeMap<Integer, LrcUnit>();
        String data = "";
        BufferedReader reader = null;
        FileInputStream fileInputStream = null;
        InputStream inputStream = null;
        HttpURLConnection httpURLConnection = null;
        String charsetName = null;
        boolean isNetworkFile = mFilePath.startsWith("http://") ? true : false;

        if (isNetworkFile) {
          externalLrcExists = false;
          isParseLrcThreadLocked = false;
          return;
          // URL url;
          // try {
          // url = new URL(mFilePath);
          // httpURLConnection = (HttpURLConnection) url.openConnection();
          // httpURLConnection.setConnectTimeout(5000);
          // httpURLConnection.setReadTimeout(5000);
          // httpURLConnection.setRequestMethod("GET");
          // int responseCode;
          // if (200 == (responseCode = httpURLConnection.getResponseCode())) {
          // Log.d(TAG, "==>mParseLrcThread----step(3)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // inputStream = httpURLConnection.getInputStream();
          // } else {
          // Log.d(TAG, "==>mParseLrcThread----step(4)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // Log.e(TAG, "connect failure! responseCode: " + responseCode);
          // if (httpURLConnection != null) {
          // httpURLConnection.disconnect();
          // httpURLConnection = null;
          // }
          // externalLrcExists = false;
          // isParseLrcThreadLocked = false;
          // return;
          // }
          // Log.d(TAG,
          // "==========================================inputStream: " +
          // inputStream);
          // Log.d(TAG, "==>mParseLrcThread----step(5)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // externalLrcExists = true;
          // charsetName = FileEncodeParser.getFileEncode(url);
          // if (this.mThreadID != mParseLrcThreadCount) {
          // Log.d(TAG, "==>mParseLrcThread----early termination: mCount=" +
          // mThreadID + ", count=" + mParseLrcThreadCount);
          // externalLrcExists = false;
          // if (null != inputStream)
          // inputStream.close();
          // isParseLrcThreadLocked = false;
          // return;
          // }
          // Log.d(TAG, "==>mParseLrcThread----step(6)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // if (charsetName == null) {
          // Log.d(TAG, "==>mParseLrcThread----step(7)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // externalLrcExists = false;
          // if (null != inputStream)
          // inputStream.close();
          // isParseLrcThreadLocked = false;
          // return;
          // }
          // Log.d(TAG, "==>mParseLrcThread----step(8)----: mCount=" + mThreadID
          // + ", count=" + mParseLrcThreadCount);
          // reader = new BufferedReader(new InputStreamReader(fileInputStream,
          // charsetName));
          // } catch (MalformedURLException e) {
          // e.printStackTrace();
          // } catch (IOException e) {
          // e.printStackTrace();
          // }
        } else {
          File file = new File(mFilePath);
          if (!file.exists()) {
            externalLrcExists = false;
            isParseLrcThreadLocked = false;
            Log.d(TAG, "Cannot find LRC: " + mFilePath);
            return;
          }
          externalLrcExists = true;
          try {
            fileInputStream = new FileInputStream(file);
            charsetName = FileEncodeParser.getFileEncode(mFilePath);
            if (this.mThreadID != mParseLrcThreadCount) {
              Log.d(TAG, "==>mParseLrcThread----early termination: mCount="
                  + mThreadID + ", count=" + mParseLrcThreadCount);
              externalLrcExists = false;
              if (null != fileInputStream)
                fileInputStream.close();
              isParseLrcThreadLocked = false;
              return;
            }
            if (charsetName == null) {
              externalLrcExists = false;
              if (null != fileInputStream)
                fileInputStream.close();
              isParseLrcThreadLocked = false;
              return;
            }
            reader = new BufferedReader(new InputStreamReader(fileInputStream,
                charsetName));
          } catch (FileNotFoundException e) {
            e.printStackTrace();
            externalLrcExists = false;
            isParseLrcThreadLocked = false;
            return;
          } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            externalLrcExists = false;
            try {
              if (null != fileInputStream)
                fileInputStream.close();
            } catch (IOException e1) {
              e1.printStackTrace();
            }
            isParseLrcThreadLocked = false;
            return;
          } catch (IOException e) {
            e.printStackTrace();
          }
        }

        if (null == reader) {
          externalLrcExists = false;
          try {
            if (null != fileInputStream)
              fileInputStream.close();

            if (null != inputStream)
              inputStream.close();
          } catch (IOException e) {
            e.printStackTrace();
          }
          isParseLrcThreadLocked = false;
          return;
        }

        try {
          while ((data = reader.readLine()) != null) {
            if (this.mThreadID != mParseLrcThreadCount) {
              Log.d(TAG, "==>mParseLrcThread----early termination: mCount="
                  + mThreadID + ", count=" + mParseLrcThreadCount);
              externalLrcExists = false;
              isParseLrcThreadLocked = false;
              return;
            }
            if (data.length() > 6) {
              if (data.charAt(3) == ':' && data.charAt(6) == '.') { // [00:00.00]
                int tempInt = -1;
                if ((tempInt = data.lastIndexOf('[')) > 0
                    && tempInt < data.length()) { // avoid multi []
                  data = data.substring(tempInt);
                }
                if (data != null) {
                  data = data.replace("[", "");
                  data = data.replace("]", "@");
                  data = data.replace(".", ":");
                  String lrc[] = data.split("@");
                  String lrcContent = null;
                  String lrcTime = null;
                  if (lrc.length == 2) {
                    lrcTime = lrc[0];
                    lrcContent = lrc[1];
                  } else {
                    lrcTime = lrc[0];
                    lrcContent = "";
                  }
                  if (lrcTime != null) {
                    String lrcTimeMember[] = lrcTime.split(":");
                    if (lrcTimeMember != null && lrcTimeMember.length == 3) {
                      int m = Integer.parseInt(lrcTimeMember[0]);
                      int s = Integer.parseInt(lrcTimeMember[1]);
                      int ms = Integer.parseInt(lrcTimeMember[2]);
                      int beginTime = (m * 60 + s) * 1000 + ms;
                      LrcUnit mLrcUnit = new LrcUnit();
                      mLrcUnit.setBeginTime(beginTime);
                      mLrcUnit.setLrcBody(lrcContent);
                      lrcTemp.put(beginTime, mLrcUnit);
                    }
                  }
                }
              }
            }
          }
        } catch (Exception e) {
          e.printStackTrace();
          externalLrcExists = false;
          isParseLrcThreadLocked = false;
          return;
        } finally {
          try {
            if (null != fileInputStream)
              fileInputStream.close();

            if (null != inputStream)
              inputStream.close();
          } catch (IOException e) {
            e.printStackTrace();
          }
        }

        Iterator<Integer> iterator = lrcTemp.keySet().iterator();
        LrcUnit currentLrcUnit = null;
        while (iterator.hasNext()) {
          if (this.mThreadID != mParseLrcThreadCount) {
            Log.d(TAG, "==>mParseLrcThread----early termination: mCount="
                + mThreadID + ", count=" + mParseLrcThreadCount);
            externalLrcExists = false;
            isParseLrcThreadLocked = false;
            return;
          }
          Object nextKey = iterator.next();
          LrcUnit nextLrcUnit = lrcTemp.get(nextKey);
          if (currentLrcUnit == null) {
            currentLrcUnit = nextLrcUnit;
          } else {
            LrcUnit tempLrcUnit = new LrcUnit();
            tempLrcUnit = currentLrcUnit;
            tempLrcUnit.setLineTime(nextLrcUnit.getBeginTime()
                - currentLrcUnit.getBeginTime());
            lrcSentenceList.add(tempLrcUnit);
            currentLrcUnit = nextLrcUnit;
          }
        }

        if (setLyricHandler != null) {
          setLyricHandler.sendEmptyMessage(0);
        }
      } // end work

      Log.d(TAG, "==>mParseLrcThread----end: mCount=" + mThreadID + ", count="
          + mParseLrcThreadCount);
      isParseLrcThreadLocked = false;
      return;
    } // end run()

  }

  @SuppressWarnings("rawtypes")
  private class AsyncUpdateAlbumTask extends AsyncTask {
    private int targetIndex;

    @Override
    protected void onPreExecute() {
      super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... params) {
      Log.d("----AsyncUpdateAlbumTask----", "doInBackground");
      this.targetIndex = (Integer) params[0];
      BitmapDrawable bmpDraw = null;
      if (MusicPlayList.sCurrentMusicList != null
          && targetMusicPlayPosition >= 0
          && targetMusicPlayPosition < MusicPlayList.sCurrentMusicList.size()
          && this.targetIndex == targetMusicPlayPosition) {
        if (MusicPlayList.sCurrentMusicList.get(targetMusicPlayPosition).embededImageExists == 1) {
          Bitmap bitmap = getArtworkFromCache(MusicPlayList.sCurrentMusicList
              .get(targetIndex).filePath);
          if (null != bitmap) {
            bmpDraw = new BitmapDrawable(bitmap);
          } else {
            if (MusicPlayList.sCurrentMusicList != null && targetIndex >= 0
                && targetIndex < MusicPlayList.sCurrentMusicList.size()) {// for
                                                                          // DMR
                                                                          // async
                                                                          // null
                                                                          // point
              bitmap = getArtwork(MusicPlayList.sCurrentMusicList
                  .get(targetIndex).filePath);
            }
            if (null != bitmap) {
              bmpDraw = new BitmapDrawable(bitmap);
            } else {
              if (MusicPlayList.sCurrentMusicList != null && targetIndex >= 0
                  && targetIndex < MusicPlayList.sCurrentMusicList.size()) {// for
                                                                            // DMR
                                                                            // async
                                                                            // null
                                                                            // point
                MusicPlayList.sCurrentMusicList.get(targetIndex).embededImageExists = 0;
              }
            }
          }
        }
      }
      return bmpDraw;
    }

    @Override
    protected void onPostExecute(Object result) {
      Log.d("----AsyncUpdateAlbumTask----", "onPostExecute");
      BitmapDrawable bmpDraw = (BitmapDrawable) result;
      if (null == bmpDraw) {
        albumImageView.setImageResource(R.drawable.music_album_default);
      } else {
        albumImageView.setImageDrawable(bmpDraw);
      }
    }

  }

  @SuppressWarnings("rawtypes")
  private class AsyncRetrieveMetaData extends AsyncTask {
    private int targetIndex;

    @Override
    protected void onPreExecute() {
      super.onPreExecute();
    }

    @Override
    protected Object doInBackground(Object... params) {
      Log.d("----AsyncRetrieveMetaData----", "doInBackground");
      this.targetIndex = (Integer) params[0];
      boolean doUpdateArtist = false;
      if (MusicPlayList.sCurrentMusicList != null
          && targetMusicPlayPosition >= 0
          && targetMusicPlayPosition < MusicPlayList.sCurrentMusicList.size()
          && this.targetIndex == targetMusicPlayPosition
          && null == MusicPlayList.sCurrentMusicList.get(targetIndex).artist) {
        doUpdateArtist = true;
        String tempPath = MusicPlayList.sCurrentMusicList.get(targetIndex).filePath;
        if (tempPath != null && !tempPath.toLowerCase().endsWith(".aac")) {
          MediaMetadataRetriever mediaMetadataRetriever = new MediaMetadataRetriever();
          String artist = null;
          try {
            mediaMetadataRetriever.setDataSource(tempPath);
            artist = mediaMetadataRetriever
                .extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
          } catch (Exception e) {
            e.printStackTrace();
          }
          if (null != artist) {
            if (MusicPlayList.sCurrentMusicList != null && targetIndex >= 0
                && targetIndex < MusicPlayList.sCurrentMusicList.size()) {// for
                                                                          // DMR
                                                                          // async
                                                                          // null
                                                                          // point
              MusicPlayList.sCurrentMusicList.get(targetIndex).artist = artist;
            }
          } else {
            if (MusicPlayList.sCurrentMusicList != null && targetIndex >= 0
                && targetIndex < MusicPlayList.sCurrentMusicList.size()) {// for
                                                                          // DMR
                                                                          // async
                                                                          // null
                                                                          // point
              MusicPlayList.sCurrentMusicList.get(targetIndex).artist = " ";
            }
          }
        } else {
          if (MusicPlayList.sCurrentMusicList != null && targetIndex >= 0
              && targetIndex < MusicPlayList.sCurrentMusicList.size()) {// for
                                                                        // DMR
                                                                        // async
                                                                        // null
                                                                        // point
            MusicPlayList.sCurrentMusicList.get(targetIndex).artist = " ";
          }
        }
      }
      return doUpdateArtist;
    }

    @Override
    protected void onPostExecute(Object result) {
      Log.d("----AsyncRetrieveMetaData----", "onPostExecute");
      boolean doUpdateArtist = (Boolean) result;
      if (doUpdateArtist && null != singerView) {
        singerView.setText(MusicPlayList.sCurrentMusicList
            .get(targetMusicPlayPosition).artist);
      }
    }

  }

  private class DMRMessageReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      if (PublicConstants.MusicCommon.INTENT_ACTION_PLAY_PREPARED
          .equals(action)) {
        if (IntentStarter.DMR_START == mPlayIntentType
            && DMRSharingInformation.sInitSeekPosition > 0)
          seekTo((int) DMRSharingInformation.sInitSeekPosition);
      } else if (PublicConstants.DMRIntent.ACTION_DMR_EVENT.equals(action)) {
        int cmd = intent
            .getIntExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD, -1);
        switch (cmd) {
        case PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY:
          play();
          if (playPauseButton != null) {
            playPauseButton.setBackgroundResource(R.drawable.pause);
          }
          // autoHideMusicControllor();
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE:
          pause();
          if (playPauseButton != null) {
            playPauseButton.setBackgroundResource(R.drawable.play);
          }
          // if (musicControllor != null) {
          // musicControllor.setVisibility(View.VISIBLE);
          // }
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK:
          int seekValue = intent.getIntExtra(
              PublicConstants.DMRIntent.KEY_SEEK_VALUE, intCurrentPosition);
          // if (musicControllor != null) {
          // musicControllor.setVisibility(View.VISIBLE);
          // }
          seekTo(seekValue);
          // if (isPlaying) {
          // autoHideMusicControllor();
          // }
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP:
          stop();
          DMRSharingInformation.resetDMRShareData();
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_EXIT:
          finish();
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME:
          if (null == CMDIntent) {
            CMDIntent = new Intent(
                PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
          }
          CMDIntent.putExtra("playerCommand", MusicConstant.CMD_SET_LR_VOLUME);
          CMDIntent.putExtra("left", DMRSharingInformation.sLeftVolume);
          CMDIntent.putExtra("right", DMRSharingInformation.sRightVolume);
          sendBroadcast(CMDIntent);
          break;

        case PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME:
          // do nothing
          break;

        default:
          break;
        }
      }
    }
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
    Dialog switchChannelDialog = new AlertDialog.Builder(this)
        .setTitle(getString(R.string.switch_channel))
        .setIcon(android.R.drawable.ic_dialog_info)
        .setSingleChoiceItems(mAudioChannels, mCurrentChannelPostion,
            mSwitchChannelClickListener)
        .setNegativeButton(getString(R.string.cancel), null).show();
    WindowManager.LayoutParams params = switchChannelDialog.getWindow()
        .getAttributes();
    params.x = 450;
    params.width = 250;
    switchChannelDialog.getWindow().setAttributes(params);
  }

  DialogInterface.OnClickListener mSwitchChannelClickListener = new DialogInterface.OnClickListener() {
    @Override
    public void onClick(DialogInterface dialog, int position) {
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

      if (null == CMDIntent) {
        CMDIntent = new Intent(
            PublicConstants.MusicCommon.INTENT_ACTION_PLAY_CMD);
      }
      CMDIntent.putExtra("playerCommand", MusicConstant.CMD_SET_CHANNEL);
      CMDIntent.putExtra("channelID", channelID);
      sendBroadcast(CMDIntent);

      mCurrentChannelPostion = position;
      dialog.dismiss();
    }
  };
}
