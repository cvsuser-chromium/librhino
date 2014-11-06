package com.hybroad.video;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.FrameLayout;
import android.widget.GridView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.hybroad.dlna.DLNAConstant;
import com.hybroad.dlna.DLNAUtils;
import com.hybroad.dlna.GlobalDMPDevice;
import com.hybroad.json.DMSFileJSON;
import com.hybroad.json.DMSFileListJSON;
import com.hybroad.json.JSONUtils;
import com.hybroad.media.DevicesList;
import com.hybroad.media.R;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.SharedPreferencesHelper;

@SuppressLint("DefaultLocale")
public class VideoBrowser extends Activity {
  private static final String TAG = "----VideoBrowser----";
  private Context mContext = VideoBrowser.this;

  private boolean isQueryCurrentDirThreadLocked = false;
  private Thread mQueryCurrentDirThread = null;
  private long mQueryCurrentDirThreadCount = 0;

  /* just cmp lowercase, so need to convert the string to lowercase */
  private String[] VIDEO_TYPES = new String[] { ".avi", ".asf", ".dat", ".flv",
      ".f4v", ".mkv", ".mov", ".mp4", ".mpg", ".mpeg", ".mpeg1", ".mpeg2",
      ".m2ts", ".rmvb", ".rm", ".ts", ".vob", ".wmv", ".3gp" };

  private boolean isRetrieveMetaDataThreadLocked = false;
  private Thread mRetrieveMetaDataThread = null;
  private long mRetrieveMetaDataThreadCount = 0;

  private boolean isRetrieveVideoFrameThreadLocked = false;
  private Thread mRetrieveVideoFrameThread = null;
  private long mRetrieveVideoFrameThreadCount = 0;

  private FrameLayout mVideoThumbnailsLayout;
  private TextView mCurrentPathView;
  private GridView mVideoGridView = null;
  private VideoThumbItemAdapter mVideoThumbItemAdapter = null;
  private int mOldFirstVisiblePosition = -1;
  private int mFirstVisibleVideo = -1;
  private int mLastVisibleVideo = -1;

  private ArrayList<VideoFile> mCurrentVideoList = null;
  private ArrayList<VideoFile> mCurrentFolderList = null;
  private ArrayList<VideoFile> mCurrentLevelTotalList = null;
  private String mDeviceType;
  private String mDeviceName;
  private final int ROOT_LEVEL = 0;
  private String mRootDirectory;
  private int mCurrentDirectoryLevel = ROOT_LEVEL;
  private String mCurrentDirectory;
  private ArrayList<String> mParentIDTree; // for DMSFile
  private String mCurrentDMSPath = ""; // for DMSFile

  private View viewFocusedBeforeMenu;
  private View mVideoExploreMenuLayoutContent;
  private View mSortMenuLayout;

  private int mCurrentSortRuleID;
  private int mDMPOrderID = DLNAConstant.Order.ASCENDING;
  private Comparator<VideoFile> mFilenameComparator;
  private Comparator<VideoFile> mDateTimeComparator;
  private Comparator<VideoFile> mSizeComparator;

  private static final int ACTIVITY_STATE_RUN = 0;
  private static final int ACTIVITY_STATE_STOP = 1;
  private int mActivityState = ACTIVITY_STATE_RUN;

  private int getActivityState() {
    int r = ACTIVITY_STATE_RUN;

    synchronized (this) {
      r = mActivityState;
    }

    return r;
  }

  private void setActivityState(int state) {
    synchronized (this) {
      mActivityState = state;
    }
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    Log.i(TAG, "onCreate");
    super.onCreate(savedInstanceState);
    setContentView(R.layout.video_browser_main);

    mCurrentSortRuleID = SharedPreferencesHelper.getValue(VideoBrowser.this,
        SharedPreferencesHelper.XML_VIDEO_EXPLORE_SETTINGS,
        SharedPreferencesHelper.KEY_VIDEO_EXPLORE_SORT_RULE,
        PublicConstants.SORT_BY_NAME);
    if (mCurrentSortRuleID < 0
        || mCurrentSortRuleID >= PublicConstants.SORT_ARRAY_LENGTH) {
      mCurrentSortRuleID = 0;
    }
    mFilenameComparator = new VideoFile().getTitleComparator();
    mDateTimeComparator = new VideoFile().getDateTimeComparator();
    mSizeComparator = new VideoFile().getSizeComparator();

    mCurrentDirectoryLevel = ROOT_LEVEL;
    if (DevicesList.s_currentDevice != null) {
      if (DevicesList.s_currentDevice.isLocalDevice()) {
        String path = null;
        if ((path = DevicesList.s_currentDevice.getRootPath()) != null) {
          mRootDirectory = path;
          mCurrentDirectory = mRootDirectory;
          mDeviceType = PublicConstants.DeviceType.LOCAL_DEVICE;
          mDeviceName = DevicesList.s_currentDevice.getName();
          startNewQueryCurrentDirThread(mCurrentDirectory);
        } else {
          finish();
        }
      } else {
        if (DevicesList.s_currentDevice.getDeviceID() != null) {
          mRootDirectory = "-1";
          mCurrentDirectory = mRootDirectory;
          mDeviceType = PublicConstants.DeviceType.DMS_DEVICE;
          mDeviceName = DevicesList.s_currentDevice.getName();
          startNewQueryCurrentDirThread(mCurrentDirectory);
        } else {
          finish();
        }
      }
    } else {
      finish();
    }
  }

  @Override
  protected void onRestart() {
    Log.i(TAG, "onRestart");
    this.setActivityState(ACTIVITY_STATE_RUN);
    super.onRestart();
  }

  @Override
  protected void onStart() {
    Log.i(TAG, "onStart");
    this.setActivityState(ACTIVITY_STATE_RUN);
    super.onStart();
  }

  @Override
  protected void onResume() {
    Log.i(TAG, "onResume");
    this.setActivityState(ACTIVITY_STATE_RUN);
    super.onResume();
    if (null != mCurrentVideoList && mCurrentVideoList.size() > 0
        && mFirstVisibleVideo >= 0) {
      startRetrieveVideoFrameThread(mFirstVisibleVideo, mLastVisibleVideo);
    }
  }

  @Override
  protected void onPause() {
    Log.i(TAG, "onPause");
    stopRetrieveVideoFrameThread();
    stopRetrieveMetaDataThread();
    this.setActivityState(ACTIVITY_STATE_STOP);
    super.onPause();
  }

  @Override
  protected void onStop() {
    Log.i(TAG, "onStop");
    stopQueryCurrentDirThread();
    // stopRetrieveVideoFrameThread();
    // stopRetrieveMetaDataThread();
    this.setActivityState(ACTIVITY_STATE_STOP);
    super.onStop();
  }

  @Override
  protected void onDestroy() {
    Log.i(TAG, "onDestroy");
    CustomWidget.releaseLoadIcon();
    super.onDestroy();
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    Log.d(TAG, "KeyEvent-->" + event.toString());
    int columnsNumber = -1;
    if (null != mVideoGridView) {
      columnsNumber = mVideoGridView.getNumColumns();
    }
    switch (keyCode) {
    case KeyEvent.KEYCODE_DPAD_UP:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        return true;
      }
      if (null != mVideoGridView
          && mVideoGridView.getSelectedItemPosition() < columnsNumber
          && mVideoGridView.getSelectedItemPosition() >= 0) {
        Intent mIntent = new Intent("SetVideoTabFocused");
        VideoBrowser.this.sendBroadcast(mIntent);
        return true;
      }
      break;

    case KeyEvent.KEYCODE_DPAD_DOWN:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        return true;
      }
      break;

    case KeyEvent.KEYCODE_DPAD_LEFT:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        return true;
      }
      if (null != mVideoGridView
          && mVideoGridView.getSelectedItemPosition() != 0
          && (mVideoGridView.getSelectedItemPosition() % columnsNumber == 0)) {
        mVideoGridView
            .setSelection(mVideoGridView.getSelectedItemPosition() - 1);
        return true;
      }
      break;

    case KeyEvent.KEYCODE_DPAD_RIGHT:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        return true;
      }
      if (null != mVideoGridView
          && mVideoGridView.getSelectedItemPosition() != (mVideoGridView
              .getCount() - 1)
          && ((mVideoGridView.getSelectedItemPosition() + 1) % columnsNumber == 0)) {
        mVideoGridView
            .setSelection(mVideoGridView.getSelectedItemPosition() + 1);
        return true;
      }
      break;

    case KeyEvent.KEYCODE_DPAD_CENTER:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        if (null != mSortMenuLayout && mSortMenuLayout.isFocused()) {
          if (null != viewFocusedBeforeMenu)
            viewFocusedBeforeMenu.requestFocus();
          mVideoExploreMenuLayoutContent.setVisibility(View.INVISIBLE);
          Dialog sortDialog = new AlertDialog.Builder(this)
              .setTitle(getString(R.string.sort))
              .setIcon(android.R.drawable.ic_dialog_info)
              .setSingleChoiceItems(R.array.sort_option_items,
                  mCurrentSortRuleID, mSortOnClickListener)
              .setNegativeButton(getString(R.string.cancel), null).show();
          WindowManager.LayoutParams params = sortDialog.getWindow()
              .getAttributes();
          params.width = 300;
          sortDialog.getWindow().setAttributes(params);
        }
        return true;
      }
      break;

    case KeyEvent.KEYCODE_BACK:
      if (null != mVideoExploreMenuLayoutContent
          && mVideoExploreMenuLayoutContent.isShown()) {
        if (viewFocusedBeforeMenu != null) {
          viewFocusedBeforeMenu.requestFocus();
        }
        mVideoExploreMenuLayoutContent.setVisibility(View.INVISIBLE);
        return true;
      }
      if (mCurrentDirectoryLevel > ROOT_LEVEL) {
        mCurrentDirectoryLevel--;
        if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
          mCurrentDirectory = mCurrentDirectory.substring(0,
              mCurrentDirectory.lastIndexOf('/'));
          startNewQueryCurrentDirThread(mCurrentDirectory);
        } else {
          if (null != mParentIDTree && mParentIDTree.size() >= 2) {
            mCurrentDirectory = mParentIDTree.get(0);
            mParentIDTree.remove(0);
            int position = mCurrentDMSPath.lastIndexOf('/');
            if (position >= 0)
              mCurrentDMSPath = mCurrentDMSPath.substring(0, position);
            startNewQueryCurrentDirThread(mCurrentDirectory);
          }
        }
        return true;
      }
      break;

    case KeyEvent.KEYCODE_MENU:
      if (null != mVideoExploreMenuLayoutContent) {
        if (mVideoExploreMenuLayoutContent.isShown()) {
          if (viewFocusedBeforeMenu != null) {
            viewFocusedBeforeMenu.requestFocus();
          }
          mVideoExploreMenuLayoutContent.setVisibility(View.INVISIBLE);
          return true;
        }

        viewFocusedBeforeMenu = getCurrentFocus();
        mVideoExploreMenuLayoutContent.setVisibility(View.VISIBLE);
        if (null != mSortMenuLayout)
          mSortMenuLayout.requestFocus();
        return true;
      }
      break;

    default:
      break;
    }

    return super.onKeyDown(keyCode, event);
  }

  @SuppressLint("HandlerLeak")
  private Handler mUpdateUIHandler = new Handler() {
    @Override
    public void handleMessage(android.os.Message msg) {
      if (getActivityState() == ACTIVITY_STATE_STOP) {
        return;
      }
      updateVideoBrowser();
    };
  };

  private void updateVideoBrowser() {
    mOldFirstVisiblePosition = -1;
    mFirstVisibleVideo = -1;
    mLastVisibleVideo = -1;
    if (mCurrentLevelTotalList != null && !mCurrentLevelTotalList.isEmpty()) {
      switchThumbnails();
      if (mCurrentVideoList != null && mCurrentVideoList.size() > 0) {
        startRetrieveVideoFrameThread(0, mCurrentVideoList.size() - 1);
      }
    } else {
      switchNoVideosNotice();
    }
    return;
  }

  private void switchThumbnails() {
    if (null == mVideoThumbnailsLayout) {
      mVideoThumbnailsLayout = (FrameLayout) VideoBrowser.this
          .getLayoutInflater().inflate(R.layout.video_browser_thumbnail, null);
      RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
          LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
      mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT,
          RelativeLayout.TRUE);
      mVideoThumbnailsLayout.setLayoutParams(mLayoutParams);

      mCurrentPathView = (TextView) mVideoThumbnailsLayout
          .findViewById(R.id.video_current_path);
      updateCurrentPath();

      mVideoGridView = (GridView) mVideoThumbnailsLayout
          .findViewById(R.id.video_gridview);
      mVideoGridView.setSelector(R.drawable.selector_2);
      mVideoGridView.setOnScrollListener(new RetrieveVideoFrameOnScroll());
      mVideoGridView.setOnItemClickListener(mOnItemClickListener);

      mVideoThumbItemAdapter = new VideoThumbItemAdapter(mContext,
          mCurrentLevelTotalList, mVideoGridView);
      mVideoGridView.setAdapter(mVideoThumbItemAdapter);

      RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.layout_playlist_mainarea);
      mainArea.removeAllViews();
      mainArea.addView(mVideoThumbnailsLayout);

      mVideoExploreMenuLayoutContent = mainArea
          .findViewById(R.id.video_explore_menu_layout_content);
      mSortMenuLayout = mainArea.findViewById(R.id.video_sort_menu_layout);
    } else {
      mVideoThumbItemAdapter.notifyDataSetChanged();

      updateCurrentPath();

      if (null != mVideoGridView && mVideoGridView.isShown()
          && mVideoGridView.isFocused()) {
        if (mCurrentDirectoryLevel > ROOT_LEVEL) {
          mVideoGridView.setSelection(1);
        } else {
          mVideoGridView.setSelection(0);
        }
      }
    }
  }

  private void switchNoVideosNotice() {
    LinearLayout noVideoLayout = (LinearLayout) VideoBrowser.this
        .getLayoutInflater().inflate(R.layout.video_browser_no_video, null);
    RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
    mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
    noVideoLayout.setLayoutParams(mLayoutParams);

    RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.layout_playlist_mainarea);
    if (mainArea.isShown()) {
      Intent mIntent = new Intent("SetVideoTabFocused");
      VideoBrowser.this.sendBroadcast(mIntent);
    }
    mainArea.removeAllViews();
    mainArea.addView(noVideoLayout);
  }

  private void updateCurrentPath() {
    if (null == mCurrentPathView)
      return;
    String currentPathText = getString(R.string.currentPath);
    if (mCurrentDirectoryLevel == ROOT_LEVEL) {
      mCurrentPathView.setText(currentPathText + mDeviceType + "/"
          + mDeviceName);
    } else if (mCurrentDirectoryLevel > ROOT_LEVEL) {
      if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
        String tempPath = String.copyValueOf(mCurrentDirectory.toCharArray());
        int position = tempPath.indexOf(mDeviceName);
        if (position >= 0 && position < tempPath.length()) {
          mCurrentPathView.setText(currentPathText + mDeviceType + "/"
              + tempPath.substring(position));
        }
      } else {
        mCurrentPathView.setText(currentPathText + mDeviceType + "/"
            + mDeviceName + mCurrentDMSPath);
      }
    }
  }

  private GridView.OnItemClickListener mOnItemClickListener = new GridView.OnItemClickListener() {
    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
      if (mCurrentVideoList != null && mCurrentVideoList.size() > 0) {
        stopRetrieveVideoFrameThread();
      }
      if (mCurrentLevelTotalList != null
          && arg2 < mCurrentLevelTotalList.size()) {
        VideoFile video = mCurrentLevelTotalList.get(arg2);
        if (video == null) {
          return;
        }
        int subdirSize = (mCurrentFolderList == null) ? 0 : mCurrentFolderList
            .size();
        if (mCurrentDirectoryLevel == ROOT_LEVEL) {
          if (arg2 < subdirSize) { // go to subdir
            mCurrentDirectoryLevel++;
            if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
              mCurrentDirectory = mCurrentLevelTotalList.get(arg2).path;
              startNewQueryCurrentDirThread(mCurrentDirectory);
            } else {
              mCurrentDirectory = mCurrentLevelTotalList.get(arg2).objectID;
              mCurrentDMSPath = mCurrentDMSPath + "/"
                  + mCurrentLevelTotalList.get(arg2).title;
              if (null == mParentIDTree) {
                mParentIDTree = new ArrayList<String>();
                mParentIDTree.add(0, "-1");
              }
              mParentIDTree.add(0, mCurrentLevelTotalList.get(arg2).parentID);
              startNewQueryCurrentDirThread(mCurrentDirectory);
            }
          } else { // go to play video
            if (video.mimeType != null && video.mimeType.equals("video/bd")) {
              BDLauncher.launchHiBDPlayer(mContext, video.path);
              return;
            }

            Intent intent = new Intent(mContext, VideoPlayer.class);
            VideoPlayList pLMediaFileList = new VideoPlayList(
                mCurrentLevelTotalList.get(arg2).path, arg2 - subdirSize);
            intent.putExtra(PublicConstants.MEDIA_FILE_LIST, pLMediaFileList);
            mContext.startActivity(intent);
          }
        } else if (mCurrentDirectoryLevel > ROOT_LEVEL) {
          if (arg2 == 0) { // back to up-level dir
            mCurrentDirectoryLevel--;
            if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
              mCurrentDirectory = mCurrentLevelTotalList.get(0).path;
              startNewQueryCurrentDirThread(mCurrentDirectory);
            } else {
              if (null != mParentIDTree && mParentIDTree.size() >= 2) {
                mCurrentDirectory = mParentIDTree.get(0);
                mParentIDTree.remove(0);
                int position = mCurrentDMSPath.lastIndexOf('/');
                if (position >= 0)
                  mCurrentDMSPath = mCurrentDMSPath.substring(0, position);
                startNewQueryCurrentDirThread(mCurrentDirectory);
              }
            }
          } else if (arg2 < subdirSize) { // go to subdir
            mCurrentDirectoryLevel++;
            if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
              mCurrentDirectory = mCurrentLevelTotalList.get(arg2).path;
              startNewQueryCurrentDirThread(mCurrentDirectory);
            } else {
              mCurrentDirectory = mCurrentLevelTotalList.get(arg2).objectID;
              mCurrentDMSPath = mCurrentDMSPath + "/"
                  + mCurrentLevelTotalList.get(arg2).title;
              if (null == mParentIDTree) {
                mParentIDTree = new ArrayList<String>();
                mParentIDTree.add(0, "-1");
              }
              mParentIDTree.add(0, mCurrentLevelTotalList.get(arg2).parentID);
              startNewQueryCurrentDirThread(mCurrentDirectory);
            }
          } else { // go to play video
            if (video.mimeType != null && video.mimeType.equals("video/bd")) {
              BDLauncher.launchHiBDPlayer(mContext, video.path);
              return;
            }

            Intent intent = new Intent(mContext, VideoPlayer.class);
            VideoPlayList pLMediaFileList = new VideoPlayList(
                mCurrentLevelTotalList.get(arg2).path, arg2 - subdirSize);
            intent.putExtra(PublicConstants.MEDIA_FILE_LIST, pLMediaFileList);
            mContext.startActivity(intent);
          }
        }
      }
    }
  };

  private class RetrieveVideoFrameOnScroll implements OnScrollListener {
    @Override
    public void onScroll(AbsListView view, int firstVisibleItem,
        int visibleItemCount, int totalItemCount) {
      Log.d(TAG, "---onScroll---");
      if (mCurrentVideoList != null && mCurrentVideoList.size() > 0) {
        int directoryCount = mCurrentFolderList.size();

        if (firstVisibleItem == 0) {
          if (visibleItemCount != 0) {
            if (firstVisibleItem != mOldFirstVisiblePosition) {
              mOldFirstVisiblePosition = firstVisibleItem;
              int start = firstVisibleItem;
              int end = firstVisibleItem + visibleItemCount - 1;
              mFirstVisibleVideo = start > directoryCount ? start
                  - directoryCount : 0;
              mLastVisibleVideo = end > directoryCount ? end - directoryCount
                  : 0;
              Log.d(TAG, "mFirstVisibleVideo: " + mFirstVisibleVideo
                  + ", mLastVisibleVideo: " + mLastVisibleVideo);
              startRetrieveVideoFrameThread(mFirstVisibleVideo,
                  mLastVisibleVideo);
            }
          }
        } else {
          if (firstVisibleItem != mOldFirstVisiblePosition) {
            mOldFirstVisiblePosition = firstVisibleItem;
            int start = firstVisibleItem;
            int end = firstVisibleItem + visibleItemCount - 1;
            mFirstVisibleVideo = start > directoryCount ? start
                - directoryCount : 0;
            mLastVisibleVideo = end > directoryCount ? end - directoryCount : 0;
            Log.d(TAG, "mFirstVisibleVideo: " + mFirstVisibleVideo
                + ", mLastVisibleVideo: " + mLastVisibleVideo);
            startRetrieveVideoFrameThread(mFirstVisibleVideo, mLastVisibleVideo);
          }
        }
      }
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
    }
  }

  private void startNewQueryCurrentDirThread(String currentDirectory) {
    CustomWidget.showLoadIcon(mContext);
    stopRetrieveMetaDataThread();
    stopRetrieveVideoFrameThread();
    stopQueryCurrentDirThread();
    mQueryCurrentDirThread = new Thread(new QueryCurrentDirRunnable(
        mQueryCurrentDirThreadCount, currentDirectory));
    mQueryCurrentDirThread.start();
  }

  private void stopQueryCurrentDirThread() {
    mQueryCurrentDirThreadCount++;
    if (mQueryCurrentDirThread != null) {
      mQueryCurrentDirThread = null;
    }
  }

  class QueryCurrentDirRunnable implements Runnable {
    private long mThreadID = 0;
    private String mTargetDirectory;

    public QueryCurrentDirRunnable(long count, String directory) {
      QueryCurrentDirRunnable.this.mThreadID = count;
      QueryCurrentDirRunnable.this.mTargetDirectory = directory;
    }

    @Override
    public void run() {
      Log.d(TAG, "==>mQueryCurrentDirThread----start: mCount=" + mThreadID
          + ", count=" + mQueryCurrentDirThreadCount);
      if (this.mThreadID != mQueryCurrentDirThreadCount) {
        Log.d(TAG, "==>mQueryCurrentDirThread----early termination: mCount="
            + mThreadID + ", count=" + mQueryCurrentDirThreadCount);
        return;
      }

      while (isQueryCurrentDirThreadLocked) {
        try {
          Log.d(TAG, "mQueryCurrentDirThread:" + this.mThreadID
              + " is locked, waitting......");
          Thread.currentThread();
          Thread.sleep(1000);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
        if (this.mThreadID != mQueryCurrentDirThreadCount) {
          Log.d(TAG, "==>mQueryCurrentDirThread----early termination: mCount="
              + mThreadID + ", count=" + mQueryCurrentDirThreadCount);
          return;
        }
      }
      isQueryCurrentDirThreadLocked = true;

      // work area
      {
        // clear old data
        if (mCurrentVideoList == null) {
          mCurrentVideoList = new ArrayList<VideoFile>();
        }
        if (mCurrentFolderList == null) {
          mCurrentFolderList = new ArrayList<VideoFile>();
        }
        mCurrentVideoList.clear();
        mCurrentFolderList.clear();
        if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
          File currentDirFile = new File(mTargetDirectory);
          File[] files = null;
          if (currentDirFile != null) {
            files = currentDirFile.listFiles();
          }
          if (files != null) {
            for (File currentFile : files) {
              if (currentFile.isDirectory()) {
                VideoFile directory = new VideoFile();
                directory.path = currentFile.getAbsolutePath();
                directory.title = currentFile.getName();
                directory.dateTime = currentFile.lastModified();
                directory.setTypeFolder();
                mCurrentFolderList.add(directory);
              } else if (isAcceptableVideo(currentFile.getName())) {
                VideoFile video = new VideoFile();
                video.path = currentFile.getAbsolutePath();
                video.title = currentFile.getName();
                video.title = video.title.substring(0,
                    video.title.lastIndexOf('.'));
                video.dateTime = currentFile.lastModified();
                video.size = currentFile.length();
                video.setTypeVideo();
                mCurrentVideoList.add(video);
              }
            }
          }
        } else if (null != GlobalDMPDevice.s_DMPDevice) {
          while (null != DevicesList.s_currentDevice) {
            String deviceID = DevicesList.s_currentDevice.getDeviceID();
            if (TextUtils.isEmpty(deviceID))
              break;

            String fileListID = GlobalDMPDevice.s_DMPDevice.openFileList(
                deviceID, mTargetDirectory, null,
                DLNAUtils.convertToDMPSortID(mCurrentSortRuleID), mDMPOrderID);
            Log.d(TAG, "------fileListID: " + fileListID);
            if (TextUtils.isEmpty(fileListID))
              break;

            int fileCount = GlobalDMPDevice.s_DMPDevice.getCount(Integer
                .parseInt(fileListID));
            Log.d(TAG, "------fileCount: " + fileCount);
            for (int i = 0; i < fileCount; i = i + 2) {
              String fileListJSON = GlobalDMPDevice.s_DMPDevice.getList(
                  Integer.parseInt(fileListID), i, 2);
              Log.d(TAG, "------fileListJSON: " + fileListJSON);
              DMSFileListJSON fileListObject = new DMSFileListJSON();
              if (!TextUtils.isEmpty(fileListJSON)) {
                fileListObject = (DMSFileListJSON) JSONUtils
                    .loadObjectFromJSON(fileListJSON, fileListObject);
              }
              ArrayList<DMSFileJSON> files = null;
              if (null != fileListObject) {
                // Log.d(TAG, "------containers count: " +
                // fileListObject.getCount());
                if (null != (files = fileListObject.getFileList())) {
                  for (DMSFileJSON file : files) {
                    // Log.d(TAG, "------getFilename: " +
                    // file.getFilename());
                    // Log.d(TAG, "------getObjectID: " +
                    // file.getObjectID());
                    // Log.d(TAG, "------getParentID: " +
                    // file.getParentID());
                    // Log.d(TAG, "------getClassID: " +
                    // file.getClassID());
                    // Log.d(TAG, "------getFilePath: " +
                    // file.getFilepath());
                    // Log.d(TAG,
                    // "=======================================================");

                    if (DLNAUtils.isContainer(file.getClassID())) {
                      VideoFile directory = new VideoFile();
                      directory.objectID = file.getObjectID();
                      directory.title = file.getFilename();
                      directory.classID = file.getClassID();
                      directory.parentID = file.getParentID();
                      directory.setTypeFolder();
                      mCurrentFolderList.add(directory);
                    } else if (DLNAConstant.Item.VIDEO_ITEM.equals(file
                        .getClassID())
                        || DLNAConstant.Item.MOVIE.equals(file.getClassID())
                        || DLNAConstant.Item.VIDEO_BROADCAST.equals(file
                            .getClassID())
                        || DLNAConstant.Item.MUSIC_VIDEO_CLIP.equals(file
                            .getClassID())
                        || DLNAConstant.Item.CPVR.equals(file.getClassID())
                        || DLNAConstant.Item.VIDEO_DOWNLOADS.equals(file
                            .getClassID())) {
                      VideoFile video = new VideoFile();
                      video.objectID = file.getObjectID();
                      video.title = file.getFilename();
                      video.classID = file.getClassID();
                      video.parentID = file.getParentID();
                      video.path = file.getFilepath();
                      video.size = file.getSize();
                      video.setTypeVideo();
                      mCurrentVideoList.add(video);
                    }
                  }
                }
              } // end if (null != containerListObject)
            } // end for (int i = 0; i < fileCount; i++ )
            break;
          } // end while (null != DevicesList.s_currentDevice)
        }

        // sort
        sortingFile(mCurrentVideoList, mCurrentSortRuleID, false);
        sortingFile(mCurrentFolderList, mCurrentSortRuleID, true);

        // add back dir to folder list
        if (mCurrentDirectoryLevel > ROOT_LEVEL) {
          if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
            if (mTargetDirectory != null) {
              VideoFile backDirectory = new VideoFile();
              if (mTargetDirectory.lastIndexOf('/') >= 0
                  && mTargetDirectory.lastIndexOf('/') < mTargetDirectory
                      .length()) {
                backDirectory.path = mTargetDirectory.substring(0,
                    mTargetDirectory.lastIndexOf('/'));
              }
              Log.d(TAG, "back dir Path: " + backDirectory.path);
              backDirectory.title = getString(R.string.backDirectory);
              backDirectory.setTypeFolder();
              mCurrentFolderList.add(0, backDirectory);
            }
          } else {
            VideoFile backDirectory = new VideoFile();
            backDirectory.title = getString(R.string.backDirectory);
            backDirectory.setTypeFolder();
            mCurrentFolderList.add(0, backDirectory);
          }
        }

        if (null == mCurrentLevelTotalList) {
          mCurrentLevelTotalList = new ArrayList<VideoFile>();
        } else {
          mCurrentLevelTotalList.clear();
        }

        // Add folder to total list
        for (int i = 0; i < mCurrentFolderList.size(); i++) {
          mCurrentLevelTotalList.add(mCurrentFolderList.get(i));
        }

        // Add video to total list
        for (int i = 0; i < mCurrentVideoList.size(); i++) {
          mCurrentLevelTotalList.add(mCurrentVideoList.get(i));
        }

        // store current mCurrentVideoList
        CommonData.setVideoList(mCurrentVideoList);

        mUpdateUIHandler.sendEmptyMessage(0);
        CustomWidget.hideLoadIcon();

        if (mCurrentVideoList != null && mCurrentVideoList.size() > 0) {
          // startNewRetrieveMetaDataThread();
        }

      } // end work

      Log.d(TAG, "==>mQueryCurrentDirThread----end: mCount=" + mThreadID
          + ", count=" + mQueryCurrentDirThreadCount);
      isQueryCurrentDirThreadLocked = false;
      return;
    } // end run()

  } // end class QueryCurrentDirRunnable

  private void startNewRetrieveMetaDataThread() {
    stopRetrieveMetaDataThread();
    mRetrieveMetaDataThread = new Thread(new RetrieveMetaDataRunnable(
        mRetrieveMetaDataThreadCount));
    mRetrieveMetaDataThread.start();
  }

  private void stopRetrieveMetaDataThread() {
    mRetrieveMetaDataThreadCount++;
    mRetrieveMetaDataThread = null;
  }

  class RetrieveMetaDataRunnable implements Runnable {
    private long mThreadID = 0;

    public RetrieveMetaDataRunnable(long count) {
      RetrieveMetaDataRunnable.this.mThreadID = count;
    }

    @Override
    public void run() {
      Log.d(TAG, "==>mRetrieveMetaDataThread----start: mCount=" + mThreadID
          + ", count=" + mRetrieveMetaDataThreadCount);
      if (this.mThreadID != mRetrieveMetaDataThreadCount) {
        Log.d(TAG, "==>mRetrieveMetaDataThread----early termination: mCount="
            + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
        return;
      }

      while (isRetrieveMetaDataThreadLocked) {
        try {
          Log.d(TAG, "mRetrieveMetaDataThread:" + this.mThreadID
              + " is locked, waitting......");
          Thread.currentThread();
          Thread.sleep(1000);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
        if (this.mThreadID != mRetrieveMetaDataThreadCount) {
          Log.d(TAG, "==>mRetrieveMetaDataThread----early termination: mCount="
              + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
          return;
        }
      }
      isRetrieveMetaDataThreadLocked = true;

      // work area
      {
        if (mCurrentVideoList != null && mCurrentVideoList.size() > 0) {
          MediaMetadataRetriever mediaMetadataRetriever = new MediaMetadataRetriever();
          // int videosCount = mCurrentVideoList.size();
          // for (int i = 0; i < videosCount; i++) {
          for (int i = 0; i < mCurrentVideoList.size(); i++) { // list
                                                               // maybe
                                                               // clear
                                                               // up
                                                               // by
                                                               // other
                                                               // thread!!!
            if (this.mThreadID != mRetrieveMetaDataThreadCount) {
              Log.d(TAG,
                  "==>mRetrieveMetaDataThread----early termination: mCount="
                      + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
              if (mediaMetadataRetriever != null) {
                mediaMetadataRetriever.release();
              }
              isRetrieveMetaDataThreadLocked = false;
              return;
            }
            String tempPath = mCurrentVideoList.get(i).path;
            if (tempPath != null) {
              try {
                mediaMetadataRetriever.setDataSource(tempPath);
                if (this.mThreadID != mRetrieveMetaDataThreadCount) {
                  Log.d(TAG,
                      "==>mRetrieveMetaDataThread----early termination: mCount="
                          + mThreadID + ", count="
                          + mRetrieveMetaDataThreadCount);
                  if (mediaMetadataRetriever != null) {
                    mediaMetadataRetriever.release();
                  }
                  isRetrieveMetaDataThreadLocked = false;
                  return;
                }
                mCurrentVideoList.get(i).mimeType = mediaMetadataRetriever
                    .extractMetadata(MediaMetadataRetriever.METADATA_KEY_MIMETYPE);
              } catch (Exception e) {
                continue;
              }
            }
          }
          if (mediaMetadataRetriever != null) {
            mediaMetadataRetriever.release();
          }
        }

      } // end work

      Log.d(TAG, "==>mRetrieveMetaDataThread----end: mCount=" + mThreadID
          + ", count=" + mRetrieveMetaDataThreadCount);
      isRetrieveMetaDataThreadLocked = false;
      return;
    } // end run()

  } // end class RetrieveMetaDataRunnable

  private void startRetrieveVideoFrameThread(int startPosition, int endPosition) {
    stopRetrieveVideoFrameThread();
    mRetrieveVideoFrameThread = new Thread(new RetrieveVideoFrameRunnable(
        startPosition, endPosition, mRetrieveVideoFrameThreadCount));
    mRetrieveVideoFrameThread.start();
  }

  private void stopRetrieveVideoFrameThread() {
    mRetrieveVideoFrameThreadCount++;
    mRetrieveVideoFrameThread = null;
  }

  class RetrieveVideoFrameRunnable implements Runnable {
    private int startPosition = -1;
    private int endPosition = -1;
    private long mCount = 0;

    public RetrieveVideoFrameRunnable(int start, int end, long count) {
      RetrieveVideoFrameRunnable.this.startPosition = start;
      RetrieveVideoFrameRunnable.this.endPosition = end;
      RetrieveVideoFrameRunnable.this.mCount = count;
    }

    @Override
    public void run() {
      if (DevicesList.s_currentDevice == null
          || DevicesList.s_currentDevice.isDMSDevice())
        return; // Provisional measures
      // Because the MediaPlayer will continuously get data if the
      // underlying acquisition fails by network.

      Log.d(TAG, "==>RetrieveVideoFrameRunnable----start: mCount=" + mCount
          + ", count=" + mRetrieveVideoFrameThreadCount);
      Log.d(TAG, "==>startPosition:" + startPosition + ", endPosition:"
          + endPosition);

      try {
        Thread.currentThread();
        Thread.sleep(1000);
        if (this.mCount != mRetrieveVideoFrameThreadCount) {
          Log.d(TAG,
              "==>RetrieveVideoFrameRunnable----early termination: mCount="
                  + mCount + ", count=" + mRetrieveVideoFrameThreadCount);
          return;
        }
      } catch (InterruptedException e1) {
        e1.printStackTrace();
      }

      while (isRetrieveVideoFrameThreadLocked) {
        try {
          Log.d(TAG, "RetrieveVideoFrameRunnable [" + mCount
              + "] is locked, waitting......");
          Thread.currentThread();
          Thread.sleep(1000);
          if (this.mCount != mRetrieveVideoFrameThreadCount) {
            Log.d(TAG,
                "==>RetrieveVideoFrameRunnable----early termination: mCount="
                    + mCount + ", count=" + mRetrieveVideoFrameThreadCount);
            return;
          }
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }

      isRetrieveVideoFrameThreadLocked = true;
      for (int i = startPosition; i >= 0 && i <= endPosition
          && i < mCurrentVideoList.size(); i++) {
        if (this.mCount != mRetrieveVideoFrameThreadCount) {
          Log.d(TAG,
              "==>RetrieveVideoFrameRunnable----early termination: mCount="
                  + mCount + ", count=" + mRetrieveVideoFrameThreadCount);
          isRetrieveVideoFrameThreadLocked = false;
          return;
        }
        try {
          String imageTag = mCurrentVideoList.get(i).path;
          Drawable cachedImage = mVideoThumbItemAdapter
              .loadCacheDrawable(imageTag);
          if (cachedImage == null) {
            mVideoThumbItemAdapter.loadVideoDrawable(i, imageTag);
          }
        } catch (Exception e) {
          continue;
        }
      } // end for()
      Log.d(TAG, "==>RetrieveVideoFrameRunnable----end: mCount=" + mCount
          + ", count=" + mRetrieveVideoFrameThreadCount);
      isRetrieveVideoFrameThreadLocked = false;
      return;
    } // end run()

  }

  private boolean isAcceptableVideo(String fileName) {
    if (TextUtils.isEmpty(fileName)) {
      return false;
    }
    for (String videoNameSuffix : VIDEO_TYPES) {
      if (fileName.toLowerCase().endsWith(videoNameSuffix)) {
        return true;
      }
    }
    return false;
  }

  DialogInterface.OnClickListener mSortOnClickListener = new DialogInterface.OnClickListener() {
    @Override
    public void onClick(DialogInterface dialog, int position) {
      // 1. save to current context
      mCurrentSortRuleID = position;
      // 2. save to xml
      SharedPreferencesHelper.setValue(VideoBrowser.this,
          SharedPreferencesHelper.XML_VIDEO_EXPLORE_SETTINGS,
          SharedPreferencesHelper.KEY_VIDEO_EXPLORE_SORT_RULE,
          mCurrentSortRuleID);
      // 3. reverse order
      switch (mCurrentSortRuleID) {
      case PublicConstants.SORT_BY_DATE:
        mDateTimeComparator = Collections.reverseOrder(mDateTimeComparator);
        break;
      case PublicConstants.SORT_BY_NAME:
        mFilenameComparator = Collections.reverseOrder(mFilenameComparator);
        break;
      case PublicConstants.SORT_BY_SIZE:
        mSizeComparator = Collections.reverseOrder(mSizeComparator);
        break;

      default:
        break;
      }
      switch (mDMPOrderID) {
      case DLNAConstant.Order.ASCENDING:
        mDMPOrderID = DLNAConstant.Order.DESCENDING;
        break;
      case DLNAConstant.Order.DESCENDING:
      default:
        mDMPOrderID = DLNAConstant.Order.ASCENDING;
        break;
      }
      // 4. refresh UI
      startNewQueryCurrentDirThread(mCurrentDirectory);
      dialog.dismiss();
    }
  };

  private void sortingFile(ArrayList<VideoFile> list, int sortRuleID,
      boolean isFolder) {
    if (list == null) {
      return;
    }
    switch (sortRuleID) {
    case PublicConstants.SORT_BY_DATE:
      Collections.sort(list, mDateTimeComparator);
      break;
    case PublicConstants.SORT_BY_SIZE:
      if (!isFolder)
        Collections.sort(list, mSizeComparator);
      else
        Collections.sort(list, mFilenameComparator);
      break;
    case PublicConstants.SORT_BY_NAME:
    default:
      Collections.sort(list, mFilenameComparator);
      break;
    }
  }
}
