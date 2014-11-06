package com.hybroad.music;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
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
import com.hybroad.util.IOUtitls;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.SharedPreferencesHelper;

public class MusicBrowser extends Activity {
    private static final String TAG = "----MusicBrowser----";
    private Activity mCurrentActivity = MusicBrowser.this;

    private boolean isQueryCurrentDirThreadLocked = false;
    private Thread mQueryCurrentDirThread = null;
    private long mQueryCurrentDirThreadCount = 0;

    private boolean isRetrieveMetaDataThreadLocked = false;
    private Thread mRetrieveMetaDataThread = null;
    private long mRetrieveMetaDataThreadCount = 0;

    /* just cmp lowercase, so need to convert the string to lowercase */
    private String[] AUDIO_TYPES = new String[] { ".mp3", ".wma", ".wav",
            ".flac", ".mid", ".aac", ".ogg", ".ape" };

    private ArrayList<MusicFile> mCurrentFolderList;
    private ArrayList<MusicFile> mCurrentLevelTotalList;
    private String mDeviceType;
    private String mDeviceName;
    private final int ROOT_LEVEL = 0;
    private String mRootDirectory;
    private int mCurrentDirectoryLevel = ROOT_LEVEL;
    private String mCurrentDirectory;
    private ArrayList<String> mParentIDTree; // for DMSFile
    private String mCurrentDMSPath = ""; // for DMSFile

    private FrameLayout mMusicThumbnailsLayout;
    private TextView mCurrentPathView;
    private GridView mMusicGridview;
    private MusicThumbItemAdapter mMusicThumbItemAdapter = null;

    private boolean isRetrieveAlbumThreadLocked = false;
    private Thread mRetrieveAlbumThread = null;
    private long mRetrieveAlbumThreadCount = 0;

    private int mOldFirstVisiblePosition = -1;
    private int mFirstVisibleMusic = -1;
    private int mLastVisibleMusic = -1;

    private View viewFocusedBeforeMenu;
    private View mMusicExploreMenuLayoutContent;
    private View mSortMenuLayout;
    private View mAddToFavoriteMenuLayout;
    private View mEnterFavoriteMenuLayout;
    private MusicFile mCurrentSelectedAudio;

    private int mCurrentSortRuleID;
    private int mDMPOrderID = DLNAConstant.Order.ASCENDING;
    private Comparator<MusicFile> mFilenameComparator;
    private Comparator<MusicFile> mDateTimeComparator;
    private Comparator<MusicFile> mSizeComparator;

    /** Called when the activity is first created. */
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "--onCreate()--");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.music_browser_main);

        mCurrentSortRuleID = SharedPreferencesHelper.getValue(
                MusicBrowser.this,
                SharedPreferencesHelper.XML_MUSIC_EXPLORE_SETTINGS,
                SharedPreferencesHelper.KEY_MUSIC_EXPLORE_SORT_RULE,
                PublicConstants.SORT_BY_NAME);
        if (mCurrentSortRuleID < 0
                || mCurrentSortRuleID >= PublicConstants.SORT_ARRAY_LENGTH) {
            mCurrentSortRuleID = 0;
        }
        mFilenameComparator = new MusicFile().getTitleComparator();
        mDateTimeComparator = new MusicFile().getDateTimeComparator();
        mSizeComparator = new MusicFile().getSizeComparator();

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
        Log.i(TAG, "--onRestart()--");
        super.onRestart();
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "--onStart--");
        super.onStart();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "--onResume--");
        super.onResume();
        if (null != MusicPlayList.sCurrentMusicList
                && MusicPlayList.sCurrentMusicList.size() > 0
                && mFirstVisibleMusic >= 0) {
            startGetAlbumCoverThread(mFirstVisibleMusic, mLastVisibleMusic);
        }
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "--onPause--");
        super.onPause();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "--onStop--");
        super.onStop();
        stopCurrentRetrieveMetaDataThread();
        stopCurrentGetAlbumCoverThread();
        stopCurrentQueryDirThread();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "--onDestroy--");
        super.onDestroy();
        CustomWidget.releaseLoadIcon();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.d(TAG, "KeyEvent-->" + event.toString());
        int columnsNumber = -1;
        if (null != mMusicGridview) {
            columnsNumber = mMusicGridview.getNumColumns();
        }
        switch (keyCode) {
        case KeyEvent.KEYCODE_DPAD_UP:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown())
                return true;
            if (null != mMusicGridview
                    && (mMusicGridview.getSelectedItemPosition() < columnsNumber)
                    && (mMusicGridview.getSelectedItemPosition() >= 0)) {
                Intent mIntent = new Intent("SetMusicTabFocused");
                MusicBrowser.this.sendBroadcast(mIntent);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_DOWN:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown()) {
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_LEFT:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown()) {
                if (null != mEnterFavoriteMenuLayout
                        && mEnterFavoriteMenuLayout.isFocused()
                        && null != mAddToFavoriteMenuLayout)
                    mAddToFavoriteMenuLayout.requestFocus();
                else if (null != mAddToFavoriteMenuLayout
                        && mAddToFavoriteMenuLayout.isFocused()
                        && null != mSortMenuLayout)
                    mSortMenuLayout.requestFocus();
                return true;
            }
            if (null != mMusicGridview
                    && mMusicGridview.getSelectedItemPosition() != 0
                    && (mMusicGridview.getSelectedItemPosition()
                            % columnsNumber == 0)) {
                mMusicGridview.setSelection(mMusicGridview
                        .getSelectedItemPosition() - 1);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_RIGHT:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown()) {
                if (null != mSortMenuLayout && mSortMenuLayout.isFocused()
                        && null != mAddToFavoriteMenuLayout)
                    mAddToFavoriteMenuLayout.requestFocus();
                else if (null != mAddToFavoriteMenuLayout
                        && mAddToFavoriteMenuLayout.isFocused()
                        && null != mEnterFavoriteMenuLayout)
                    mEnterFavoriteMenuLayout.requestFocus();
                return true;
            }
            if (null != mMusicGridview
                    && mMusicGridview.getSelectedItemPosition() != (mMusicGridview
                            .getCount() - 1)
                    && ((mMusicGridview.getSelectedItemPosition() + 1)
                            % columnsNumber == 0)) {
                mMusicGridview.setSelection(mMusicGridview
                        .getSelectedItemPosition() + 1);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_CENTER:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown()) {
                if (null != mSortMenuLayout && mSortMenuLayout.isFocused()) {
                    if (null != viewFocusedBeforeMenu)
                        viewFocusedBeforeMenu.requestFocus();
                    mMusicExploreMenuLayoutContent
                            .setVisibility(View.INVISIBLE);
                    Dialog sortDialog = new AlertDialog.Builder(this)
                            .setTitle(getString(R.string.sort))
                            .setIcon(android.R.drawable.ic_dialog_info)
                            .setSingleChoiceItems(R.array.sort_option_items,
                                    mCurrentSortRuleID, mSortOnClickListener)
                            .setNegativeButton(getString(R.string.cancel), null)
                            .show();
                    WindowManager.LayoutParams params = sortDialog.getWindow()
                            .getAttributes();
                    params.width = 300;
                    sortDialog.getWindow().setAttributes(params);
                } else if (null != mAddToFavoriteMenuLayout
                        && mAddToFavoriteMenuLayout.isFocused()) {
                    if (null != viewFocusedBeforeMenu)
                        viewFocusedBeforeMenu.requestFocus();
                    mMusicExploreMenuLayoutContent
                            .setVisibility(View.INVISIBLE);
                    if (null != mCurrentSelectedAudio
                            && (mCurrentSelectedAudio instanceof MusicFile)
                            && mCurrentSelectedAudio.isMusic())
                        addToFavorite();
                } else if (null != mEnterFavoriteMenuLayout
                        && mEnterFavoriteMenuLayout.isFocused()) {
                    if (null != viewFocusedBeforeMenu)
                        viewFocusedBeforeMenu.requestFocus();
                    mMusicExploreMenuLayoutContent
                            .setVisibility(View.INVISIBLE);
                    enterMyFavorite();
                }
                return true;
            }
            break;

        case KeyEvent.KEYCODE_BACK:
            if (null != mMusicExploreMenuLayoutContent
                    && mMusicExploreMenuLayoutContent.isShown()) {
                if (viewFocusedBeforeMenu != null) {
                    viewFocusedBeforeMenu.requestFocus();
                }
                mMusicExploreMenuLayoutContent.setVisibility(View.INVISIBLE);
                return true;
            }
            if (mCurrentDirectoryLevel > ROOT_LEVEL
                    && mCurrentLevelTotalList != null
                    && mCurrentLevelTotalList.size() > 0) {
                mCurrentDirectoryLevel--;
                if (PublicConstants.DeviceType.LOCAL_DEVICE.equals(mDeviceType)) {
                    mCurrentDirectory = mCurrentLevelTotalList.get(0).filePath;
                    startNewQueryCurrentDirThread(mCurrentDirectory);
                } else {
                    if (null != mParentIDTree && mParentIDTree.size() >= 2) {
                        mCurrentDirectory = mParentIDTree.get(0);
                        mParentIDTree.remove(0);
                        int position = mCurrentDMSPath.lastIndexOf('/');
                        if (position >= 0)
                            mCurrentDMSPath = mCurrentDMSPath.substring(0,
                                    position);
                        startNewQueryCurrentDirThread(mCurrentDirectory);
                    }
                }
                return true;
            }
            break;

        case KeyEvent.KEYCODE_MENU:
            if (null != mMusicExploreMenuLayoutContent) {
                if (mMusicExploreMenuLayoutContent.isShown()) {
                    if (viewFocusedBeforeMenu != null) {
                        viewFocusedBeforeMenu.requestFocus();
                    }
                    mMusicExploreMenuLayoutContent
                            .setVisibility(View.INVISIBLE);
                    return true;
                }

                mCurrentSelectedAudio = null;
                if (null != mMusicGridview)
                    mCurrentSelectedAudio = (MusicFile) mMusicGridview
                            .getSelectedItem();

                viewFocusedBeforeMenu = getCurrentFocus();

                if (null != mCurrentSelectedAudio
                        && (mCurrentSelectedAudio instanceof MusicFile)
                        && mCurrentSelectedAudio.isMusic()) {
                    mAddToFavoriteMenuLayout.setVisibility(View.VISIBLE);
                    mEnterFavoriteMenuLayout.setVisibility(View.VISIBLE);
                    mMusicExploreMenuLayoutContent.setVisibility(View.VISIBLE);
                    if (null != mSortMenuLayout)
                        mSortMenuLayout.requestFocus();
                } else {
                    mAddToFavoriteMenuLayout.setVisibility(View.GONE);
                    mEnterFavoriteMenuLayout.setVisibility(View.GONE);
                    mMusicExploreMenuLayoutContent.setVisibility(View.VISIBLE);
                    if (null != mSortMenuLayout)
                        mSortMenuLayout.requestFocus();
                }
                return true;
            }
            break;

        default:
            break;
        }

        return super.onKeyDown(keyCode, event);
    }

    private Handler mUpdateUIHandler = new Handler() {
        public void handleMessage(Message msg) {
            updateMusicBrower();
        }
    };

    private void updateMusicBrower() {
        mOldFirstVisiblePosition = -1;
        mFirstVisibleMusic = -1;
        mLastVisibleMusic = -1;
        if (mCurrentLevelTotalList != null && !mCurrentLevelTotalList.isEmpty()) {
            switchThumbnails();
            if (MusicPlayList.sCurrentMusicList != null
                    && MusicPlayList.sCurrentMusicList.size() > 0) {
                startGetAlbumCoverThread(0,
                        MusicPlayList.sCurrentMusicList.size() - 1);
            }
        } else {
            switchNoMusicNotice();
        }
    }

    private void switchThumbnails() {
        if (null == mMusicThumbnailsLayout) {
            mMusicThumbnailsLayout = (FrameLayout) MusicBrowser.this
                    .getLayoutInflater().inflate(
                            R.layout.music_browser_thumbnail, null);
            RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
                    LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
            mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT,
                    RelativeLayout.TRUE);
            mMusicThumbnailsLayout.setLayoutParams(mLayoutParams);

            mCurrentPathView = (TextView) mMusicThumbnailsLayout
                    .findViewById(R.id.music_current_path);
            updateCurrentPath();

            mMusicGridview = (GridView) mMusicThumbnailsLayout
                    .findViewById(R.id.audio_gridview);
            mMusicGridview.setSelector(R.drawable.selector_3);
            mMusicGridview
                    .setOnScrollListener(new RetrieveAlbumCoverOnScroll());
            mMusicGridview.setOnItemClickListener(mOnItemClickListener);

            mMusicThumbItemAdapter = new MusicThumbItemAdapter(
                    mCurrentActivity, mCurrentLevelTotalList, mMusicGridview);
            mMusicGridview.setAdapter(mMusicThumbItemAdapter);

            RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.musics_playlist_mainarea);
            mainArea.removeAllViews();
            mainArea.addView(mMusicThumbnailsLayout);

            mMusicExploreMenuLayoutContent = mainArea
                    .findViewById(R.id.music_explore_menu_layout_content);
            mSortMenuLayout = mainArea
                    .findViewById(R.id.music_sort_menu_layout);
            mAddToFavoriteMenuLayout = mainArea
                    .findViewById(R.id.add_to_favorite_menu_layout);
            mEnterFavoriteMenuLayout = mainArea
                    .findViewById(R.id.enter_favorite_menu_layout);
        } else {
            mMusicThumbItemAdapter.notifyDataSetChanged();

            updateCurrentPath();

            if (null != mMusicGridview && mMusicGridview.isShown()
                    && mMusicGridview.isFocused()) {
                if (mCurrentDirectoryLevel > ROOT_LEVEL) {
                    mMusicGridview.setSelection(1);
                } else {
                    mMusicGridview.setSelection(0);
                }
            }
        }
    }

    private void switchNoMusicNotice() {
        LinearLayout noAudioLayout = (LinearLayout) MusicBrowser.this
                .getLayoutInflater().inflate(R.layout.music_browser_no_music,
                        null);
        RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
                LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
        mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT,
                RelativeLayout.TRUE);
        noAudioLayout.setLayoutParams(mLayoutParams);

        RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.musics_playlist_mainarea);
        if (mainArea.isShown()) {
            mainArea.setFocusable(true);
            mainArea.setFocusableInTouchMode(true);
            mainArea.requestFocus();
            Intent mIntent = new Intent("SetMusicTabFocused");
            MusicBrowser.this.sendBroadcast(mIntent);
        }
        mainArea.removeAllViews();
        mainArea.addView(noAudioLayout);
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
                String tempPath = String.copyValueOf(mCurrentDirectory
                        .toCharArray());
                int position = tempPath.indexOf(mDeviceName);
                if (position >= 0 && position < tempPath.length()) {
                    mCurrentPathView.setText(currentPathText + mDeviceType
                            + "/" + tempPath.substring(position));
                }
            } else {
                mCurrentPathView.setText(currentPathText + mDeviceType + "/"
                        + mDeviceName + mCurrentDMSPath);
            }
        }
    }

    private GridView.OnItemClickListener mOnItemClickListener = new GridView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
            if (MusicPlayList.sCurrentMusicList != null
                    && MusicPlayList.sCurrentMusicList.size() > 0) {
                stopCurrentGetAlbumCoverThread();
                stopCurrentRetrieveMetaDataThread();
            }
            if (mCurrentLevelTotalList != null
                    && arg2 < mCurrentLevelTotalList.size()) {
                int subdirSize = (mCurrentFolderList == null) ? 0
                        : mCurrentFolderList.size();
                if (mCurrentDirectoryLevel == ROOT_LEVEL) {
                    if (arg2 < subdirSize) { // go to subdir
                        mCurrentDirectoryLevel++;
                        if (PublicConstants.DeviceType.LOCAL_DEVICE
                                .equals(mDeviceType)) {
                            mCurrentDirectory = mCurrentLevelTotalList
                                    .get(arg2).filePath;
                            startNewQueryCurrentDirThread(mCurrentDirectory);
                        } else {
                            mCurrentDirectory = mCurrentLevelTotalList
                                    .get(arg2).objectID;
                            mCurrentDMSPath = mCurrentDMSPath + "/"
                                    + mCurrentLevelTotalList.get(arg2).title;
                            if (null == mParentIDTree) {
                                mParentIDTree = new ArrayList<String>();
                                mParentIDTree.add(0, "-1");
                            }
                            mParentIDTree.add(0,
                                    mCurrentLevelTotalList.get(arg2).parentID);
                            startNewQueryCurrentDirThread(mCurrentDirectory);
                        }
                    } else { // go to play music
                        Intent startPlayIntent = new Intent();
                        startPlayIntent.setClass(mCurrentActivity,
                                MusicPlayer.class);
                        startPlayIntent
                                .putExtra(
                                        PublicConstants.IntentStarter.NORMAL_START_MUSIC_KEY,
                                        PublicConstants.IntentStarter.NORMAL_START_MUSIC_NAME);
                        startPlayIntent.putExtra(
                                PublicConstants.SELECTED_ITEM_POSITION, arg2
                                        - subdirSize);
                        startPlayIntent.putExtra(MusicConstant.PLAYLIST_TYPE,
                                MusicConstant.PLAYLIST_TYPE_NORMAL);
                        startActivity(startPlayIntent);
                    }
                } else if (mCurrentDirectoryLevel > ROOT_LEVEL) {
                    if (arg2 == 0) { // back to up-level dir
                        mCurrentDirectoryLevel--;
                        if (PublicConstants.DeviceType.LOCAL_DEVICE
                                .equals(mDeviceType)) {
                            mCurrentDirectory = mCurrentLevelTotalList.get(0).filePath;
                            startNewQueryCurrentDirThread(mCurrentDirectory);
                        } else {
                            if (null != mParentIDTree
                                    && mParentIDTree.size() >= 2) {
                                mCurrentDirectory = mParentIDTree.get(0);
                                mParentIDTree.remove(0);
                                int position = mCurrentDMSPath.lastIndexOf('/');
                                if (position >= 0)
                                    mCurrentDMSPath = mCurrentDMSPath
                                            .substring(0, position);
                                startNewQueryCurrentDirThread(mCurrentDirectory);
                            }
                        }
                    } else if (arg2 < subdirSize) { // go to subdir
                        mCurrentDirectoryLevel++;
                        if (PublicConstants.DeviceType.LOCAL_DEVICE
                                .equals(mDeviceType)) {
                            mCurrentDirectory = mCurrentLevelTotalList
                                    .get(arg2).filePath;
                            startNewQueryCurrentDirThread(mCurrentDirectory);
                        } else {
                            mCurrentDirectory = mCurrentLevelTotalList
                                    .get(arg2).objectID;
                            mCurrentDMSPath = mCurrentDMSPath + "/"
                                    + mCurrentLevelTotalList.get(arg2).title;
                            if (null == mParentIDTree) {
                                mParentIDTree = new ArrayList<String>();
                                mParentIDTree.add(0, "-1");
                            }
                            mParentIDTree.add(0,
                                    mCurrentLevelTotalList.get(arg2).parentID);
                            startNewQueryCurrentDirThread(mCurrentDirectory);
                        }
                    } else { // go to play music
                        Intent startPlayIntent = new Intent();
                        startPlayIntent.setClass(mCurrentActivity,
                                MusicPlayer.class);
                        startPlayIntent
                                .putExtra(
                                        PublicConstants.IntentStarter.NORMAL_START_MUSIC_KEY,
                                        PublicConstants.IntentStarter.NORMAL_START_MUSIC_NAME);
                        startPlayIntent.putExtra(
                                PublicConstants.SELECTED_ITEM_POSITION, arg2
                                        - subdirSize);
                        startPlayIntent.putExtra(MusicConstant.PLAYLIST_TYPE,
                                MusicConstant.PLAYLIST_TYPE_NORMAL);
                        startActivity(startPlayIntent);
                    }
                }
            }
        }
    };

    private class RetrieveAlbumCoverOnScroll implements OnScrollListener {
        @Override
        public void onScroll(AbsListView view, int firstVisibleItem,
                int visibleItemCount, int totalItemCount) {
            if (MusicPlayList.sCurrentMusicList != null
                    && MusicPlayList.sCurrentMusicList.size() > 0) {
                int directoryCount = mCurrentFolderList.size();

                if (firstVisibleItem == 0) {
                    if (visibleItemCount != 0) {
                        if (firstVisibleItem != mOldFirstVisiblePosition) {
                            mOldFirstVisiblePosition = firstVisibleItem;
                            int start = firstVisibleItem;
                            int end = firstVisibleItem + visibleItemCount - 1;
                            mFirstVisibleMusic = start > directoryCount ? start
                                    - directoryCount : 0;
                            mLastVisibleMusic = end > directoryCount ? end
                                    - directoryCount : 0;
                            Log.d(TAG, "mFirstVisibleMusic: "
                                    + mFirstVisibleMusic
                                    + ", mLastVisibleMusic: "
                                    + mLastVisibleMusic);
                            startGetAlbumCoverThread(mFirstVisibleMusic,
                                    mLastVisibleMusic);
                        }
                    }
                } else {
                    if (firstVisibleItem != mOldFirstVisiblePosition) {
                        mOldFirstVisiblePosition = firstVisibleItem;
                        int start = firstVisibleItem;
                        int end = firstVisibleItem + visibleItemCount - 1;
                        mFirstVisibleMusic = start > directoryCount ? start
                                - directoryCount : 0;
                        mLastVisibleMusic = end > directoryCount ? end
                                - directoryCount : 0;
                        Log.d(TAG, "mFirstVisibleMusic: " + mFirstVisibleMusic
                                + ", mLastVisibleMusic: " + mLastVisibleMusic);
                        startGetAlbumCoverThread(mFirstVisibleMusic,
                                mLastVisibleMusic);
                    }
                }
            }
        }

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
        }
    } // end class RetrieveAlbumCoverOnScroll

    private void enterMyFavorite() {
        File directory = new File(
                PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH);
        File[] files = null;
        if (directory.exists() && directory.isDirectory())
            files = directory.listFiles();
        if (null == files || 0 == files.length) {
            // no favorite song
            Dialog favoriteDialog = new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.favorite_is_empty))
                    .setIcon(android.R.drawable.ic_dialog_info)
                    .setPositiveButton(getString(R.string.confirm), null)
                    .show();
            WindowManager.LayoutParams params = favoriteDialog.getWindow()
                    .getAttributes();
            params.width = 300;
            favoriteDialog.getWindow().setAttributes(params);
        } else {
            stopCurrentRetrieveMetaDataThread();
            stopCurrentGetAlbumCoverThread();
            CustomWidget.showLoadIcon(mCurrentActivity);
            MusicPlayList.sCurrentMusicList = null;
            MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
            for (File file : files) {
                if (file.isFile()) {
                    MusicFile audio = new MusicFile();
                    audio.filePath = file.getAbsolutePath();
                    audio.title = file.getName();
                    audio.title = audio.title.substring(0,
                            audio.title.lastIndexOf('.'));
                    audio.setTypeMusic();
                    MusicPlayList.sCurrentMusicList.add(audio);
                }
            }
            Intent startPlayIntent = new Intent();
            startPlayIntent.setClass(mCurrentActivity, MusicPlayer.class);
            startPlayIntent.putExtra(
                    PublicConstants.IntentStarter.NORMAL_START_MUSIC_KEY,
                    PublicConstants.IntentStarter.NORMAL_START_MUSIC_NAME);
            startPlayIntent.putExtra(PublicConstants.SELECTED_ITEM_POSITION, 0);
            startPlayIntent.putExtra(MusicConstant.PLAYLIST_TYPE,
                    MusicConstant.PLAYLIST_TYPE_FAVORITE);
            startActivity(startPlayIntent);
            CustomWidget.hideLoadIcon();
        }
    }

    private void addToFavorite() {
        File file = new File(PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH);
        if (!file.exists())
            IOUtitls.copyFile(mCurrentSelectedAudio.filePath,
                    PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH,
                    mCurrentSelectedAudio.title);
        else {
            if (!file.isDirectory())
                IOUtitls.copyFile(mCurrentSelectedAudio.filePath,
                        PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH,
                        mCurrentSelectedAudio.title);
            else {
                String[] fileNames = file.list();
                if (null == fileNames
                        || fileNames.length < PublicConstants.MusicCommon.MUSIC_FAVORITE_MAX_NUMBER)
                    IOUtitls.copyFile(mCurrentSelectedAudio.filePath,
                            PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH,
                            mCurrentSelectedAudio.title);
                else {
                    Dialog favoriteDialog = new AlertDialog.Builder(this)
                            .setTitle(getString(R.string.favorite_is_full))
                            .setIcon(android.R.drawable.ic_dialog_info)
                            .setPositiveButton(getString(R.string.confirm),
                                    null).show();
                    WindowManager.LayoutParams params = favoriteDialog
                            .getWindow().getAttributes();
                    params.width = 300;
                    favoriteDialog.getWindow().setAttributes(params);
                }
            }
        }
    }

    private void startNewQueryCurrentDirThread(String currentDirectory) {
        CustomWidget.showLoadIcon(mCurrentActivity);
        stopCurrentGetAlbumCoverThread();
        stopCurrentRetrieveMetaDataThread();
        stopCurrentQueryDirThread();
        mQueryCurrentDirThread = new Thread(new QueryCurrentDirRunnable(
                mQueryCurrentDirThreadCount, currentDirectory));
        mQueryCurrentDirThread.start();
    }

    private void stopCurrentQueryDirThread() {
        mQueryCurrentDirThreadCount++;
        mQueryCurrentDirThread = null;
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
            Log.d(TAG, "==>mQueryCurrentDirThread----start: mCount="
                    + mThreadID + ", count=" + mQueryCurrentDirThreadCount);
            if (this.mThreadID != mQueryCurrentDirThreadCount) {
                Log.d(TAG,
                        "==>mQueryCurrentDirThread----early termination: mCount="
                                + mThreadID + ", count="
                                + mQueryCurrentDirThreadCount);
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
                    Log.d(TAG,
                            "==>mQueryCurrentDirThread----early termination: mCount="
                                    + mThreadID + ", count="
                                    + mQueryCurrentDirThreadCount);
                    return;
                }
            }
            isQueryCurrentDirThreadLocked = true;

            // work area
            {
                // clear old data
                if (MusicPlayList.sCurrentMusicList == null) {
                    MusicPlayList.sCurrentMusicList = new ArrayList<MusicFile>();
                }
                if (mCurrentFolderList == null) {
                    mCurrentFolderList = new ArrayList<MusicFile>();
                }
                MusicPlayList.sCurrentMusicList.clear();
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
                                MusicFile directory = new MusicFile();
                                directory.filePath = currentFile
                                        .getAbsolutePath();
                                // Log.d(TAG, "new subdir Path: " +
                                // directory.filePath);
                                directory.title = currentFile.getName();
                                // Log.d(TAG, "new subdir name: " +
                                // directory.title);
                                directory.dateTime = currentFile.lastModified();
                                directory.setTypeFolder();
                                mCurrentFolderList.add(directory);
                            } else if (isAcceptableAudio(currentFile.getName())) {
                                MusicFile music = new MusicFile();
                                music.filePath = currentFile.getAbsolutePath();
                                // Log.d(TAG, "new audio Path: " +
                                // audio.filePath);
                                music.title = currentFile.getName();
                                music.title = music.title.substring(0,
                                        music.title.lastIndexOf('.'));
                                // Log.d(TAG, "new audio name: " + audio.title);
                                music.dateTime = currentFile.lastModified();
                                music.size = currentFile.length();
                                music.setTypeMusic();
                                MusicPlayList.sCurrentMusicList.add(music);
                            }
                        }
                    }
                } else if (null != GlobalDMPDevice.s_DMPDevice) {
                    while (null != DevicesList.s_currentDevice) {
                        String deviceID = DevicesList.s_currentDevice
                                .getDeviceID();
                        if (TextUtils.isEmpty(deviceID))
                            break;

                        String fileListID = GlobalDMPDevice.s_DMPDevice
                                .openFileList(
                                        deviceID,
                                        mTargetDirectory,
                                        null,
                                        DLNAUtils
                                                .convertToDMPSortID(mCurrentSortRuleID),
                                        mDMPOrderID);
                        Log.d(TAG, "------fileListID: " + fileListID);
                        if (TextUtils.isEmpty(fileListID))
                            break;

                        int fileCount = GlobalDMPDevice.s_DMPDevice
                                .getCount(Integer.parseInt(fileListID));
                        Log.d(TAG, "------fileCount: " + fileCount);
                        for (int i = 0; i < fileCount; i = i + 2) {
                            String fileListJSON = GlobalDMPDevice.s_DMPDevice
                                    .getList(Integer.parseInt(fileListID), i, 2);
                            Log.d(TAG, "------fileListJSON: " + fileListJSON);
                            DMSFileListJSON fileListObject = new DMSFileListJSON();
                            if (!TextUtils.isEmpty(fileListJSON)) {
                                fileListObject = (DMSFileListJSON) JSONUtils
                                        .loadObjectFromJSON(fileListJSON,
                                                fileListObject);
                            }
                            ArrayList<DMSFileJSON> files = null;
                            if (null != fileListObject) {
                                // Log.d(TAG, "------containers count: " +
                                // fileListObject.getCount());
                                if (null != (files = fileListObject
                                        .getFileList())) {
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

                                        if (DLNAUtils.isContainer(file
                                                .getClassID())) {
                                            MusicFile directory = new MusicFile();
                                            directory.objectID = file
                                                    .getObjectID();
                                            directory.title = file
                                                    .getFilename();
                                            directory.classID = file
                                                    .getClassID();
                                            directory.parentID = file
                                                    .getParentID();
                                            directory.setTypeFolder();
                                            mCurrentFolderList.add(directory);
                                        } else if (DLNAConstant.Item.AUDIO_ITEM
                                                .equals(file.getClassID())
                                                || DLNAConstant.Item.MUSIC_TRACK
                                                        .equals(file
                                                                .getClassID())
                                                || DLNAConstant.Item.AUDIO_BROADCAST
                                                        .equals(file
                                                                .getClassID())
                                                || DLNAConstant.Item.AUDIO_BOOK
                                                        .equals(file
                                                                .getClassID())) {
                                            MusicFile music = new MusicFile();
                                            music.objectID = file.getObjectID();
                                            music.title = file.getFilename();
                                            music.classID = file.getClassID();
                                            music.parentID = file.getParentID();
                                            music.filePath = file.getFilepath();
                                            music.size = file.getSize();
                                            music.setTypeMusic();
                                            MusicPlayList.sCurrentMusicList
                                                    .add(music);
                                        }
                                    }
                                }
                            } // end if (null != containerListObject)
                        } // end for (int i = 0; i < fileCount; i++ )
                        break;
                    } // end while (null != DevicesList.s_currentDevice)
                }

                // sort
                sortingFile(MusicPlayList.sCurrentMusicList,
                        mCurrentSortRuleID, false);
                sortingFile(mCurrentFolderList, mCurrentSortRuleID, true);

                // save sCurrentMusicList
                MusicPlayList.sCurrentMusicListBackup = MusicPlayList.sCurrentMusicList;

                // add back dir to folder list
                if (mCurrentDirectoryLevel > ROOT_LEVEL) {
                    if (PublicConstants.DeviceType.LOCAL_DEVICE
                            .equals(mDeviceType)) {
                        MusicFile backDirectory = new MusicFile();
                        backDirectory.filePath = mTargetDirectory.substring(0,
                                mTargetDirectory.lastIndexOf('/'));
                        Log.d(TAG, "back dir Path: " + backDirectory.filePath);
                        backDirectory.title = MusicBrowser.this
                                .getString(R.string.backDirectory);
                        backDirectory.setTypeFolder();
                        mCurrentFolderList.add(0, backDirectory);
                    } else {
                        MusicFile backDirectory = new MusicFile();
                        backDirectory.title = MusicBrowser.this
                                .getString(R.string.backDirectory);
                        backDirectory.setTypeFolder();
                        mCurrentFolderList.add(0, backDirectory);
                    }
                }

                if (null == mCurrentLevelTotalList)
                    mCurrentLevelTotalList = new ArrayList<MusicFile>();
                else
                    mCurrentLevelTotalList.clear();

                // Add folder to total list
                for (int i = 0; i < mCurrentFolderList.size(); i++) {
                    mCurrentLevelTotalList.add(mCurrentFolderList.get(i));
                }

                // Add music to total list
                for (int i = 0; i < MusicPlayList.sCurrentMusicList.size(); i++) {
                    mCurrentLevelTotalList.add(MusicPlayList.sCurrentMusicList
                            .get(i));
                }

                mUpdateUIHandler.sendEmptyMessage(0);
                CustomWidget.hideLoadIcon();

                if (MusicPlayList.sCurrentMusicList != null
                        && MusicPlayList.sCurrentMusicList.size() > 0) {
                    startNewRetrieveMetaDataThread();
                }

            } // end work

            Log.d(TAG, "==>mQueryCurrentDirThread----end: mCount=" + mThreadID
                    + ", count=" + mQueryCurrentDirThreadCount);
            isQueryCurrentDirThreadLocked = false;
            return;
        } // end run()

    } // end class QueryCurrentDirRunnable

    private void startNewRetrieveMetaDataThread() {
        stopCurrentRetrieveMetaDataThread();
        mRetrieveMetaDataThread = new Thread(new RetrieveMetaDataRunnable(
                mRetrieveMetaDataThreadCount));
        mRetrieveMetaDataThread.start();
    }

    private void stopCurrentRetrieveMetaDataThread() {
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
            Log.d(TAG, "==>mRetrieveMetaDataThread----start: mCount="
                    + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
            if (this.mThreadID != mRetrieveMetaDataThreadCount) {
                Log.d(TAG,
                        "==>mRetrieveMetaDataThread----early termination: mCount="
                                + mThreadID + ", count="
                                + mRetrieveMetaDataThreadCount);
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
                    Log.d(TAG,
                            "==>mRetrieveMetaDataThread----early termination: mCount="
                                    + mThreadID + ", count="
                                    + mRetrieveMetaDataThreadCount);
                    return;
                }
            }
            isRetrieveMetaDataThreadLocked = true;

            // work area
            {
                if (MusicPlayList.sCurrentMusicList != null
                        && MusicPlayList.sCurrentMusicList.size() > 0) {
                    MediaMetadataRetriever mediaMetadataRetriever = new MediaMetadataRetriever();
                    // int audiosCount = MusicPlayList.sCurrentMusicList.size();
                    // for (int i = 0; i < audiosCount; i++) {
                    for (int i = 0; i < MusicPlayList.sCurrentMusicList.size(); i++) { // list
                                                                                       // maybe
                                                                                       // clear
                                                                                       // up
                                                                                       // by
                                                                                       // other
                                                                                       // thread!!!
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
                        String tempPath = MusicPlayList.sCurrentMusicList
                                .get(i).filePath;
                        if (tempPath != null) {
                            if (tempPath.endsWith(".aac")
                                    || tempPath.endsWith(".AAC")) { // aac
                                                                    // consume
                                                                    // too
                                                                    // much
                                                                    // time
                                continue;
                            }
                            try {
                                mediaMetadataRetriever.setDataSource(tempPath);
                                if (this.mThreadID != mRetrieveMetaDataThreadCount) {
                                    Log.d(TAG,
                                            "==>mRetrieveMetaDataThread----early termination: mCount="
                                                    + mThreadID
                                                    + ", count="
                                                    + mRetrieveMetaDataThreadCount);
                                    if (mediaMetadataRetriever != null) {
                                        mediaMetadataRetriever.release();
                                    }
                                    isRetrieveMetaDataThreadLocked = false;
                                    return;
                                }
                                MusicPlayList.sCurrentMusicList.get(i).artist = mediaMetadataRetriever
                                        .extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
                            } catch (Exception e) {
                                e.printStackTrace();
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

    private void startGetAlbumCoverThread(int startPosition, int endPosition) {
        stopCurrentGetAlbumCoverThread();
        mRetrieveAlbumThread = new Thread(new RetrieveAlbumRunnable(
                startPosition, endPosition, mRetrieveAlbumThreadCount));
        mRetrieveAlbumThread.start();
    }

    private void stopCurrentGetAlbumCoverThread() {
        mRetrieveAlbumThreadCount++;
        mRetrieveAlbumThread = null;
    }

    class RetrieveAlbumRunnable implements Runnable {
        private int startPosition = -1;
        private int endPosition = -1;
        private long mCount = 0;

        public RetrieveAlbumRunnable(int start, int end, long count) {
            RetrieveAlbumRunnable.this.startPosition = start;
            RetrieveAlbumRunnable.this.endPosition = end;
            RetrieveAlbumRunnable.this.mCount = count;
        }

        @Override
        public void run() {
            Log.d(TAG, "==>RetrieveAlbumRunnable----start: mCount=" + mCount
                    + ", count=" + mRetrieveAlbumThreadCount);
            Log.d(TAG, "==>startPosition:" + startPosition + ", endPosition:"
                    + endPosition);

            try {
                Thread.currentThread();
                Thread.sleep(1000);
                if (this.mCount != mRetrieveAlbumThreadCount) {
                    Log.d(TAG,
                            "==>RetrieveAlbumRunnable----early termination: mCount="
                                    + mCount + ", count="
                                    + mRetrieveAlbumThreadCount);
                    return;
                }
            } catch (InterruptedException e1) {
                e1.printStackTrace();
            }

            while (isRetrieveAlbumThreadLocked) {
                try {
                    Log.d(TAG, "RetrieveAlbumRunnable [" + mCount
                            + "] is locked, waitting......");
                    Thread.currentThread();
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if (this.mCount != mRetrieveAlbumThreadCount) {
                    Log.d(TAG,
                            "==>RetrieveAlbumRunnable----early termination: mCount="
                                    + mCount + ", count="
                                    + mRetrieveAlbumThreadCount);
                    return;
                }
            }

            isRetrieveAlbumThreadLocked = true;
            for (int i = startPosition; i >= 0 && i <= endPosition
                    && i < MusicPlayList.sCurrentMusicList.size(); i++) {
                if (this.mCount != mRetrieveAlbumThreadCount) {
                    Log.d(TAG,
                            "==>RetrieveAlbumRunnable----early termination: mCount="
                                    + mCount + ", count="
                                    + mRetrieveAlbumThreadCount);
                    isRetrieveAlbumThreadLocked = false;
                    return;
                }
                try {
                    if (0 == MusicPlayList.sCurrentMusicList.get(i).embededImageExists) {
                        // Log.d(TAG, "no embededImage: " + i);
                        continue;
                    }
                    String imageTag = MusicPlayList.sCurrentMusicList.get(i).filePath;
                    Drawable cachedImage = mMusicThumbItemAdapter
                            .loadCacheDrawable(imageTag);
                    if (cachedImage == null) {
                        mMusicThumbItemAdapter.loadSongDrawable(i, imageTag);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    continue;
                }
            } // end for()
            Log.d(TAG, "==>RetrieveAlbumRunnable----end: mCount=" + mCount
                    + ", count=" + mRetrieveAlbumThreadCount);
            isRetrieveAlbumThreadLocked = false;
            return;
        } // end run()

    } // end class RetrieveAlbumRunnable

    private boolean isAcceptableAudio(String fileName) {
        if (TextUtils.isEmpty(fileName)) {
            return false;
        }
        for (String audioNameSuffix : AUDIO_TYPES) {
            if (fileName.toLowerCase().endsWith(audioNameSuffix)) {
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
            SharedPreferencesHelper.setValue(MusicBrowser.this,
                    SharedPreferencesHelper.XML_MUSIC_EXPLORE_SETTINGS,
                    SharedPreferencesHelper.KEY_MUSIC_EXPLORE_SORT_RULE,
                    mCurrentSortRuleID);
            // 3. reverse order
            switch (mCurrentSortRuleID) {
            case PublicConstants.SORT_BY_DATE:
                mDateTimeComparator = Collections
                        .reverseOrder(mDateTimeComparator);
                break;
            case PublicConstants.SORT_BY_NAME:
                mFilenameComparator = Collections
                        .reverseOrder(mFilenameComparator);
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

    private void sortingFile(ArrayList<MusicFile> list, int sortRuleID,
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