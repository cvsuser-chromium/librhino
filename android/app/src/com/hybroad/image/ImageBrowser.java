package com.hybroad.image;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.media.ExifInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.text.format.DateFormat;
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

public class ImageBrowser extends Activity {

    private static final String TAG = "ImageBrowser";
    private Activity mCurrentActivity = this;

    private boolean isQueryCurrentDirThreadLocked = false;
    private Thread mQueryCurrentDirThread = null;
    private long mQueryCurrentDirThreadCount = 0;

    private boolean isRetrieveMetaDataThreadLocked = false;
    private Thread mRetrieveMetaDataThread = null;
    private long mRetrieveMetaDataThreadCount = 0;

    /* just cmp lowercase, so need to convert the string to lowercase */
    private String[] IMAGE_SUPPORT_TYPES = new String[] { ".jpg", ".jpeg", ".png",
            ".bmp", ".tif", ".tiff",
    // ".gif"
    };

    private ArrayList<ImageFile> mCurrentFolderList;
    private ArrayList<ImageFile> mCurrentLevelTotalList;
    private String mDeviceType;
    private String mDeviceName;
    private static final int ROOT_LEVEL = 0;
    private String mRootDirectory;
    private int mCurrentDirectoryLevel = ROOT_LEVEL;
    private String mCurrentDirectory;
    private ArrayList<String> mParentIDTree; // for DMSFile
    private String mCurrentDMSPath = ""; // for DMSFile

    private FrameLayout mImageThumbnailsLayout;
    private TextView mCurrentPathView;
    private GridView mImageGridview;
    private ImageThumbItemAdapter mImageThumbItemAdapter;

    private boolean isRetrieveImageThumbnailThreadLocked = false;
    private Thread mRetrieveImageThumbnailThread = null;
    private long mRetrieveImageThumbnailThreadCount = 0;

    private int mOldFirstVisiblePosition = -1;
    private int mFirstVisibleImage = -1;
    private int mLastVisibleImage = -1;

    private View viewFocusedBeforeMenu;
    private View mImageExploreMenuLayoutContent;
    private View mSortMenuLayout;

    private int mCurrentSortRuleID;
    private int mDMPOrderID = DLNAConstant.Order.ASCENDING;
    private Comparator<ImageFile> mFilenameComparator;
    private Comparator<ImageFile> mDateTimeComparator;
    private Comparator<ImageFile> mSizeComparator;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "--onCreate()--");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.image_browser_main);

        // the id of image current sort rule 
        mCurrentSortRuleID = SharedPreferencesHelper.getValue(
                ImageBrowser.this,
                SharedPreferencesHelper.XML_IMAGE_EXPLORE_SETTINGS,
                SharedPreferencesHelper.KEY_IMAGE_EXPLORE_SORT_RULE,
                PublicConstants.SORT_BY_NAME);
        if (mCurrentSortRuleID < 0
                || mCurrentSortRuleID >= PublicConstants.SORT_ARRAY_LENGTH) {
            mCurrentSortRuleID = 0;
        }
        mFilenameComparator = new ImageFile().getTitleComparator();
        mDateTimeComparator = new ImageFile().getDateTimeComparator();
        mSizeComparator = new ImageFile().getSizeComparator();

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
        if (ImagePlayList.sCurrentImageList != null
                && ImagePlayList.sCurrentImageList.size() > 0
                && mFirstVisibleImage >= 0) {
            startRetrieveImageThumbnailThread(mFirstVisibleImage,
                    mLastVisibleImage);
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
        stopRetrieveImageThumbnailThread();
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
        if (null != mImageGridview) {
            columnsNumber = mImageGridview.getNumColumns();
        }
        switch (keyCode) {
        case KeyEvent.KEYCODE_DPAD_UP:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown())
                return true;
            if (mImageGridview != null
                    && (mImageGridview.getSelectedItemPosition() < columnsNumber)
                    && (mImageGridview.getSelectedItemPosition() >= 0)) {
                Intent mIntent = new Intent("SetPhotoTabFocused");
                ImageBrowser.this.sendBroadcast(mIntent);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_DOWN:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown()) {
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_LEFT:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown()) {
                return true;
            }
            if (null != mImageGridview
                    && mImageGridview.getSelectedItemPosition() != 0
                    && (mImageGridview.getSelectedItemPosition()
                            % columnsNumber == 0)) {
                mImageGridview.setSelection(mImageGridview
                        .getSelectedItemPosition() - 1);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_RIGHT:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown()) {
                return true;
            }
            if (null != mImageGridview
                    && mImageGridview.getSelectedItemPosition() != (mImageGridview
                            .getCount() - 1)
                    && ((mImageGridview.getSelectedItemPosition() + 1)
                            % columnsNumber == 0)) {
                mImageGridview.setSelection(mImageGridview
                        .getSelectedItemPosition() + 1);
                return true;
            }
            break;

        case KeyEvent.KEYCODE_DPAD_CENTER:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown()) {
                if (null != mSortMenuLayout && mSortMenuLayout.isFocused()) {
                    if (null != viewFocusedBeforeMenu)
                        viewFocusedBeforeMenu.requestFocus();
                    mImageExploreMenuLayoutContent
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
                }
                return true;
            }
            break;

        case KeyEvent.KEYCODE_BACK:
            if (null != mImageExploreMenuLayoutContent
                    && mImageExploreMenuLayoutContent.isShown()) {
                if (viewFocusedBeforeMenu != null) {
                    viewFocusedBeforeMenu.requestFocus();
                }
                mImageExploreMenuLayoutContent.setVisibility(View.INVISIBLE);
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
            if (null != mImageExploreMenuLayoutContent) {
                if (mImageExploreMenuLayoutContent.isShown()) {
                    if (viewFocusedBeforeMenu != null) {
                        viewFocusedBeforeMenu.requestFocus();
                    }
                    mImageExploreMenuLayoutContent
                            .setVisibility(View.INVISIBLE);
                    return true;
                }

                viewFocusedBeforeMenu = getCurrentFocus();
                mImageExploreMenuLayoutContent.setVisibility(View.VISIBLE);
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
        public void handleMessage(Message msg) {
            updateImageBrower();
        }
    };

    private void updateImageBrower() {
        mOldFirstVisiblePosition = -1;
        mFirstVisibleImage = -1;
        mLastVisibleImage = -1;
        if (mCurrentLevelTotalList != null && !mCurrentLevelTotalList.isEmpty()) {
            switchThumbnails();
            if (ImagePlayList.sCurrentImageList != null
                    && ImagePlayList.sCurrentImageList.size() > 0) {
                startRetrieveImageThumbnailThread(0,
                        ImagePlayList.sCurrentImageList.size() - 1);
            }
        } else {
            switchNoImageNotice();
        }
    }

    private void switchThumbnails() {
        if (null == mImageThumbnailsLayout) {
            mImageThumbnailsLayout = (FrameLayout) ImageBrowser.this
                    .getLayoutInflater().inflate(
                            R.layout.image_browser_thumbnail, null);
            RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
                    LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
            mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT,
                    RelativeLayout.TRUE);
            mImageThumbnailsLayout.setLayoutParams(mLayoutParams);

            mCurrentPathView = (TextView) mImageThumbnailsLayout
                    .findViewById(R.id.picture_current_path);
            updateCurrentPath();

            mImageGridview = (GridView) mImageThumbnailsLayout
                    .findViewById(R.id.picture_gridview);
            mImageGridview.setSelector(R.drawable.selector_3);
            mImageGridview
                    .setOnScrollListener(new RetrieveImageThumbnailOnScroll());
            mImageGridview.setOnItemClickListener(mOnItemClickListener);

            mImageThumbItemAdapter = new ImageThumbItemAdapter(
                    mCurrentActivity, mCurrentLevelTotalList, mImageGridview);
            mImageGridview.setAdapter(mImageThumbItemAdapter);

            RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.picture_layout_mainarea);
            mainArea.removeAllViews();
            mainArea.addView(mImageThumbnailsLayout);

            mImageExploreMenuLayoutContent = mainArea
                    .findViewById(R.id.image_explore_menu_layout_content);
            mSortMenuLayout = mainArea
                    .findViewById(R.id.image_sort_menu_layout);
        } else {
            mImageThumbItemAdapter.notifyDataSetChanged();

            updateCurrentPath();

            if (null != mImageGridview && mImageGridview.isShown()
                    && mImageGridview.isFocused()) {
                if (mCurrentDirectoryLevel > ROOT_LEVEL) {
                    mImageGridview.setSelection(1);
                } else {
                    mImageGridview.setSelection(0);
                }
            }
        }
    }

    private void switchNoImageNotice() {
        LinearLayout noImageLayout = (LinearLayout) ImageBrowser.this
                .getLayoutInflater().inflate(R.layout.image_browser_no_image,
                        null);
        RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT,
                RelativeLayout.TRUE);
        noImageLayout.setLayoutParams(mLayoutParams);

        RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.picture_layout_mainarea);
        if (mainArea.isShown()) {
            mainArea.setFocusable(true);
            mainArea.setFocusableInTouchMode(true);
            mainArea.requestFocus();
            Intent mIntent = new Intent("SetPhotoTabFocused");
            ImageBrowser.this.sendBroadcast(mIntent);
        }
        mainArea.removeAllViews();
        mainArea.addView(noImageLayout);
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
            if (ImagePlayList.sCurrentImageList != null
                    && ImagePlayList.sCurrentImageList.size() > 0) {
                stopRetrieveImageThumbnailThread();
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
                    } else { // go to show current image
                        Intent startPlayIntent = new Intent();
                        startPlayIntent.setClass(mCurrentActivity,
                                ImagePlayer.class);
                        startPlayIntent
                                .putExtra(
                                        PublicConstants.IntentStarter.NORMAL_START_IMAGE_KEY,
                                        PublicConstants.IntentStarter.NORMAL_START_IMAGE_NAME);
                        startPlayIntent.putExtra(
                                PublicConstants.SELECTED_ITEM_POSITION, arg2
                                        - subdirSize);
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
                    } else { // go to show current image
                        Intent startPlayIntent = new Intent();
                        startPlayIntent.setClass(mCurrentActivity,
                                ImagePlayer.class);
                        startPlayIntent
                                .putExtra(
                                        PublicConstants.IntentStarter.NORMAL_START_IMAGE_KEY,
                                        PublicConstants.IntentStarter.NORMAL_START_IMAGE_NAME);
                        startPlayIntent.putExtra(
                                PublicConstants.SELECTED_ITEM_POSITION, arg2
                                        - subdirSize);
                        startActivity(startPlayIntent);
                    }
                }
            }
        }
    };

    private class RetrieveImageThumbnailOnScroll implements OnScrollListener {
        @Override
        public void onScroll(AbsListView view, int firstVisibleItem,
                int visibleItemCount, int totalItemCount) {
            if (ImagePlayList.sCurrentImageList != null
                    && ImagePlayList.sCurrentImageList.size() > 0) {
                int directoryCount = mCurrentFolderList.size();
                if (firstVisibleItem == 0) {
                    if (visibleItemCount != 0) {
                        if (firstVisibleItem != mOldFirstVisiblePosition) {
                            mOldFirstVisiblePosition = firstVisibleItem;
                            int start = firstVisibleItem;
                            int end = firstVisibleItem + visibleItemCount - 1;
                            mFirstVisibleImage = start > directoryCount ? start
                                    - directoryCount : 0;
                            mLastVisibleImage = end > directoryCount ? end
                                    - directoryCount : 0;
                            Log.d(TAG, "mFirstVisibleImage: "
                                    + mFirstVisibleImage
                                    + ", mLastVisibleImage: "
                                    + mLastVisibleImage);
                            startRetrieveImageThumbnailThread(
                                    mFirstVisibleImage, mLastVisibleImage);
                        }
                    }
                } else {
                    if (firstVisibleItem != mOldFirstVisiblePosition) {
                        mOldFirstVisiblePosition = firstVisibleItem;
                        int start = firstVisibleItem;
                        int end = firstVisibleItem + visibleItemCount - 1;
                        mFirstVisibleImage = start > directoryCount ? start
                                - directoryCount : 0;
                        mLastVisibleImage = end > directoryCount ? end
                                - directoryCount : 0;
                        Log.d(TAG, "mFirstVisibleImage: " + mFirstVisibleImage
                                + ", mLastVisibleImage: " + mLastVisibleImage);
                        startRetrieveImageThumbnailThread(mFirstVisibleImage,
                                mLastVisibleImage);
                    }
                }
            }
        }

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {

        }
    }

    private void startRetrieveImageThumbnailThread(int startPosition,
            int endPosition) {
        stopRetrieveImageThumbnailThread();
        mRetrieveImageThumbnailThread = new Thread(
                new RetrieveImageThumbnailRunnable(startPosition, endPosition,
                        mRetrieveImageThumbnailThreadCount));
        mRetrieveImageThumbnailThread.start();
    }

    private void stopRetrieveImageThumbnailThread() {
        mRetrieveImageThumbnailThreadCount++;
        mRetrieveImageThumbnailThread = null;
    }

    class RetrieveImageThumbnailRunnable implements Runnable {
        private int startPosition = -1;
        private int endPosition = -1;
        private long mCount = 0;

        public RetrieveImageThumbnailRunnable(int start, int end, long count) {
            RetrieveImageThumbnailRunnable.this.startPosition = start;
            RetrieveImageThumbnailRunnable.this.endPosition = end;
            RetrieveImageThumbnailRunnable.this.mCount = count;
        }

        @Override
        public void run() {
            Log.d(TAG, "==>RetrieveImageThumbnailRunnable----start: mCount="
                    + mCount + ", count=" + mRetrieveImageThumbnailThreadCount);
            Log.d(TAG, "==>startPosition:" + startPosition + ", endPosition:"
                    + endPosition);

            try {
                Thread.currentThread();
                Thread.sleep(500);
                if (this.mCount != mRetrieveImageThumbnailThreadCount) {
                    Log.d(TAG,
                            "==>RetrieveImageThumbnailRunnable----early termination: mCount="
                                    + mCount + ", count="
                                    + mRetrieveImageThumbnailThreadCount);
                    return;
                }
            } catch (InterruptedException e1) {
                e1.printStackTrace();
            }

            while (isRetrieveImageThumbnailThreadLocked) {
                try {
                    Log.d(TAG, "RetrieveImageThumbnailRunnable [" + mCount
                            + "] is locked, waitting......");
                    Thread.currentThread();
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if (this.mCount != mRetrieveImageThumbnailThreadCount) {
                    Log.d(TAG,
                            "==>RetrieveImageThumbnailRunnable----early termination: mCount="
                                    + mCount + ", count="
                                    + mRetrieveImageThumbnailThreadCount);
                    return;
                }
            }

            isRetrieveImageThumbnailThreadLocked = true;
            if (ImagePlayList.sCurrentImageList != null) {
                for (int i = startPosition; i >= 0 && i <= endPosition
                        && i < ImagePlayList.sCurrentImageList.size(); i++) {
                    if (this.mCount != mRetrieveImageThumbnailThreadCount) {
                        Log.d(TAG,
                                "==>RetrieveImageThumbnailRunnable----early termination: mCount="
                                        + mCount + ", count="
                                        + mRetrieveImageThumbnailThreadCount);
                        isRetrieveImageThumbnailThreadLocked = false;
                        return;
                    }
                    try {
                        String imageTag = ImagePlayList.sCurrentImageList
                                .get(i).filePath;
                        Drawable cachedImage = mImageThumbItemAdapter
                                .loadCacheDrawable(imageTag);
                        if (cachedImage == null) {
                            mImageThumbItemAdapter.loadPhotoDrawable(i,
                                    imageTag);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        continue;
                    }
                } // end for()
            }
            Log.d(TAG, "==>RetrieveImageThumbnailRunnable----end: mCount="
                    + mCount + ", count=" + mRetrieveImageThumbnailThreadCount);
            isRetrieveImageThumbnailThreadLocked = false;
            return;
        } // end run()

    } // end class RetrieveAlbumRunnable

    private void startNewQueryCurrentDirThread(String currentDirectory) {
        CustomWidget.showLoadIcon(mCurrentActivity);
        stopCurrentQueryDirThread();
        stopCurrentRetrieveMetaDataThread();
        stopRetrieveImageThumbnailThread();
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
                    Log.d(TAG,
                            "mQueryCurrentDirThread is locked, waitting......");
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
                if (ImagePlayList.sCurrentImageList == null) {
                    ImagePlayList.sCurrentImageList = new ArrayList<ImageFile>();
                }
                if (mCurrentFolderList == null) {
                    mCurrentFolderList = new ArrayList<ImageFile>();
                }
                ImagePlayList.sCurrentImageList.clear();
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
                                ImageFile directory = new ImageFile();
                                directory.filePath = currentFile
                                        .getAbsolutePath();
                                directory.title = currentFile.getName();
                                directory.dateTime = currentFile.lastModified();
                                directory.setTypeFolder();
                                mCurrentFolderList.add(directory);
                            } else if (isAcceptableImage(currentFile.getName())) {
                                ImageFile image = new ImageFile();
                                image.filePath = currentFile.getAbsolutePath();
                                image.title = currentFile.getName();
                                image.title = image.title.substring(0,
                                        image.title.lastIndexOf('.'));
                                image.dateTime = currentFile.lastModified();
                                image.size = currentFile.length();
                                image.setTypeImage();
                                ImagePlayList.sCurrentImageList.add(image);
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
                                            ImageFile directory = new ImageFile();
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
                                        } else if (DLNAConstant.Item.IMAGE_ITEM
                                                .equals(file.getClassID())
                                                || DLNAConstant.Item.PHOTO
                                                        .equals(file
                                                                .getClassID())) {
                                            ImageFile image = new ImageFile();
                                            image.objectID = file.getObjectID();
                                            image.title = file.getFilename();
                                            image.classID = file.getClassID();
                                            image.parentID = file.getParentID();
                                            image.filePath = file.getFilepath();
                                            image.size = file.getSize();
                                            image.setTypeImage();
                                            ImagePlayList.sCurrentImageList
                                                    .add(image);
                                        }
                                    }
                                }
                            } // end if (null != containerListObject)
                        } // end for (int i = 0; i < fileCount; i++ )
                        break;
                    } // end while (null != DevicesList.s_currentDevice)
                }

                // sort
                sortingFile(ImagePlayList.sCurrentImageList,
                        mCurrentSortRuleID, false);
                sortingFile(mCurrentFolderList, mCurrentSortRuleID, true);

                // save sCurrentImageList
                ImagePlayList.sCurrentImageListBackup = ImagePlayList.sCurrentImageList;

                // add back dir to folder list
                if (mCurrentDirectoryLevel > ROOT_LEVEL) {
                    if (PublicConstants.DeviceType.LOCAL_DEVICE
                            .equals(mDeviceType)) {
                        ImageFile backDirectory = new ImageFile();
                        backDirectory.filePath = mTargetDirectory.substring(0,
                                mTargetDirectory.lastIndexOf('/'));
                        Log.d(TAG, "back dir Path: " + backDirectory.filePath);
                        backDirectory.title = ImageBrowser.this
                                .getString(R.string.backDirectory);
                        backDirectory.setTypeFolder();
                        mCurrentFolderList.add(0, backDirectory);
                    } else {
                        ImageFile backDirectory = new ImageFile();
                        backDirectory.title = ImageBrowser.this
                                .getString(R.string.backDirectory);
                        backDirectory.setTypeFolder();
                        mCurrentFolderList.add(0, backDirectory);
                    }
                }

                if (null == mCurrentLevelTotalList)
                    mCurrentLevelTotalList = new ArrayList<ImageFile>();
                else
                    mCurrentLevelTotalList.clear();

                // Add folder to total list
                for (int i = 0; i < mCurrentFolderList.size(); i++) {
                    mCurrentLevelTotalList.add(mCurrentFolderList.get(i));
                }

                // Add image to total list
                for (int i = 0; i < ImagePlayList.sCurrentImageList.size(); i++) {
                    mCurrentLevelTotalList.add(ImagePlayList.sCurrentImageList
                            .get(i));
                }

                mUpdateUIHandler.sendEmptyMessage(0);
                CustomWidget.hideLoadIcon();

                if (ImagePlayList.sCurrentImageList != null
                        && ImagePlayList.sCurrentImageList.size() > 0) {
                    startNewRetrieveMetaDataThread();
                }
            }

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
                if (ImagePlayList.sCurrentImageList != null
                        && ImagePlayList.sCurrentImageList.size() > 0) {
                    // int imagesCount = ImagePlayList.sCurrentImageList.size();
                    // for (int i = 0; i < imagesCount; i++) {
                    for (int i = 0; i < ImagePlayList.sCurrentImageList.size(); i++) { // list
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
                            isRetrieveMetaDataThreadLocked = false;
                            return;
                        }
                        String tempPath = ImagePlayList.sCurrentImageList
                                .get(i).filePath;
                        if (tempPath != null) {
                            try {
                                File imageFile = new File(tempPath);
                                if (imageFile != null) {
                                    ImagePlayList.sCurrentImageList.get(i).dateTaken = getTakenTimeFromImage(
                                            tempPath, imageFile.lastModified());
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                                continue;
                            }
                        }
                    }

                    /*
                     * MediaMetadataRetriever mediaMetadataRetriever = new
                     * MediaMetadataRetriever(); int imagesCount =
                     * ImagePlayList.sCurrentImageList.size(); for (int i = 0; i
                     * < imagesCount; i++) { if (this.mThreadID !=
                     * mRetrieveMetaDataThreadCount) { Log.d(TAG,
                     * "==>mRetrieveMetaDataThread----early termination: mCount="
                     * + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
                     * if (mediaMetadataRetriever != null) {
                     * mediaMetadataRetriever.release(); }
                     * isRetrieveMetaDataThreadLocked = false; return; } String
                     * tempPath =
                     * ImagePlayList.sCurrentImageList.get(i).filePath; if
                     * (tempPath != null) { try {
                     * mediaMetadataRetriever.setDataSource(tempPath); if
                     * (this.mThreadID != mRetrieveMetaDataThreadCount) {
                     * Log.d(TAG,
                     * "==>mRetrieveMetaDataThread----early termination: mCount="
                     * + mThreadID + ", count=" + mRetrieveMetaDataThreadCount);
                     * if (mediaMetadataRetriever != null) {
                     * mediaMetadataRetriever.release(); }
                     * isRetrieveMetaDataThreadLocked = false; return; }
                     * ImagePlayList.sCurrentImageList.get(i).date =
                     * mediaMetadataRetriever
                     * .extractMetadata(MediaMetadataRetriever
                     * .METADATA_KEY_DATE); } catch (Exception e) { continue; }
                     * } } if (mediaMetadataRetriever != null) {
                     * mediaMetadataRetriever.release(); }
                     */
                }

            } // end work

            Log.d(TAG, "==>mRetrieveMetaDataThread----end: mCount=" + mThreadID
                    + ", count=" + mRetrieveMetaDataThreadCount);
            isRetrieveMetaDataThreadLocked = false;
            return;
        } // end run()

    } // end class RetrieveMetaDataRunnable

    private boolean isAcceptableImage(String fileName) {
        if (TextUtils.isEmpty(fileName)) {
            return false;
        }
        for (String imageNameSuffix : IMAGE_SUPPORT_TYPES) {
            if (fileName.toLowerCase().endsWith(imageNameSuffix)) {
                return true;
            }
        }
        return false;
    }

    private String getTakenTimeFromImage(String filePath, long defaultTime) {
        String defaultDate = DateFormat.format("yyyy/MM/dd", defaultTime)
                .toString();
        // if (filePath.endsWith(".jpg") || filePath.endsWith(".JPG") ||
        // filePath.endsWith(".jpeg") || filePath.endsWith(".JPEG")) {
        ExifInterface exif = null;
        try {
            Log.d(TAG, "to create exif from " + filePath);
            exif = new ExifInterface(filePath);
        } catch (IOException ex) {
            // exif is null
            return defaultDate;
        }
        if (exif != null) {
            String date = exif.getAttribute(ExifInterface.TAG_DATETIME);
            if (date != null) {
                // dateTaken: yyyy:mm:dd hh:mm:ss
                date = date.substring(0, 10);
                defaultDate = date.replace(':', '/');
            }
        }
        // }
        return defaultDate;
    }

    DialogInterface.OnClickListener mSortOnClickListener = new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int position) {
            // 1. save to current context
            mCurrentSortRuleID = position;
            // 2. save to xml
            SharedPreferencesHelper.setValue(ImageBrowser.this,
                    SharedPreferencesHelper.XML_IMAGE_EXPLORE_SETTINGS,
                    SharedPreferencesHelper.KEY_IMAGE_EXPLORE_SORT_RULE,
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

    private void sortingFile(ArrayList<ImageFile> list, int sortRuleID,
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