package com.hybroad.media;

import android.app.ActivityGroup;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hybroad.dlna.DMPMessageTransit;
import com.hybroad.dlna.OnCurrentDMSChanged;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.PublicConstants.ActivityState;
import com.hybroad.util.StatusBarOperator;

@SuppressWarnings("deprecation")
public class MediaFileManager extends ActivityGroup {
    private final String TAG = "----MediaFileManager----";

    private boolean isShowVideoPage = false;
    private boolean isShowMusicPage = false;
    private boolean isShowPhotoPage = false;

    private View tempView;
    private TextView videoTab;
    private TextView musicTab;
    private TextView photoTab;

    public LinearLayout pageContainer;
    private Intent[] pageIntents;
    private setFocusBroadcastReceiver mSetFocusBroadcastReceiver;
    private StorageManager mStorageManager;
    private DMPMessageTransit mDMPMessageTransit;
    private int mActivityState = ActivityState.UNKNOWN;

    /** Called when the activity is first created. */
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "---onCreate()---");
        super.onCreate(savedInstanceState);
        mActivityState = ActivityState.ON_CREATE;
        StatusBarOperator.hideStatusBar(this);
        setContentView(R.layout.file_manager_main);

        registerReceiver();

        initView();

        if (null != DevicesList.s_currentDevice) {
            if (DevicesList.s_currentDevice.isLocalDevice()) {
                mStorageManager = (StorageManager) MediaFileManager.this
                        .getSystemService(STORAGE_SERVICE);
//                mStorageManager.registerListener(mStorageEventListener);
            } else {
                mDMPMessageTransit = new DMPMessageTransit(
                        new OnCurrentDMSChanged() {
                            @Override
                            public void onDMSDeviceDown() {
                                if (ActivityState.ON_RESUME == mActivityState) {
                                    CustomWidget
                                            .toastDeviceDown(MediaFileManager.this);
                                }
                                finish();
                            }
                        });
                IntentFilter filter = new IntentFilter(
                        PublicConstants.DMPIntent.ACTION_DMS_DEVICE_CHANGE);
                registerReceiver(mDMPMessageTransit, filter);
            }
        }
    }

    @Override
    protected void onRestart() {
        Log.i(TAG, "---onRestart()---");
        super.onRestart();
        mActivityState = ActivityState.ON_RESTART;
    }

    @Override
    protected void onStart() {
        Log.i(TAG, "---onStart()---");
        super.onStart();
        mActivityState = ActivityState.ON_START;
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "---onResume()---");
        super.onResume();
        mActivityState = ActivityState.ON_RESUME;
        getLocalActivityManager().dispatchResume();

        // kill one of 'android.process.media'
        // ProcessUtils.killProcessByPackage(this,
        // "com.android.providers.media");
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "---onPause()---");
        super.onPause();
        mActivityState = ActivityState.ON_PAUSE;
        getLocalActivityManager().dispatchPause(isFinishing());
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "---onStop()---");
        super.onStop();
        mActivityState = ActivityState.ON_STOP;
        getLocalActivityManager().dispatchStop();
    }

    protected void onDestroy() {
        Log.i(TAG, "---onDestroy()---");
        super.onDestroy();
        mActivityState = ActivityState.ON_DESTROY;
        getLocalActivityManager().dispatchDestroy(isFinishing());

        if (null != mSetFocusBroadcastReceiver) {
            unregisterReceiver(mSetFocusBroadcastReceiver);
        }

        if (null != mStorageManager) {
//            mStorageManager.unregisterListener(mStorageEventListener);
        }

        if (null != mDMPMessageTransit) {
            unregisterReceiver(mDMPMessageTransit);
        }
    }

    private void registerReceiver() {
        mSetFocusBroadcastReceiver = new setFocusBroadcastReceiver();
        IntentFilter setFocusFilter = new IntentFilter();
        setFocusFilter.addAction("SetVideoTabFocused");
        setFocusFilter.addAction("SetMusicTabFocused");
        setFocusFilter.addAction("SetPhotoTabFocused");
        registerReceiver(mSetFocusBroadcastReceiver, setFocusFilter);
    }

    private void initView() {
        videoTab = (TextView) findViewById(R.id.tab_video);
        final Drawable videoTabBackgroundDrawable = videoTab.getBackground();
        musicTab = (TextView) findViewById(R.id.tab_music);
        final Drawable musicTabBackgroundDrawable = musicTab.getBackground();
        photoTab = (TextView) findViewById(R.id.tab_photo);
        final Drawable photoTabBackgroundDrawable = photoTab.getBackground();
        pageContainer = (LinearLayout) findViewById(R.id.pageContainer);

        videoTab.setOnFocusChangeListener(new OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                Log.i(TAG, "videoTextView----hasFocus:" + hasFocus
                        + ",isfocused:" + videoTab.isFocused());
                if (hasFocus) {
                    if (!isShowVideoPage) {
                        musicTab.setBackgroundDrawable(musicTabBackgroundDrawable);
                        photoTab.setBackgroundDrawable(photoTabBackgroundDrawable);
                        switchPageWindow(0);
                        isShowVideoPage = true;
                        isShowPhotoPage = false;
                        isShowMusicPage = false;
                        Log.i(TAG, "switchPageWindow(0)");
                    }
                } else {
                    if (isShowVideoPage) {
                        videoTab.setBackgroundResource(R.drawable.tab_selected);
                    }
                }
            }
        });
        videoTab.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (!isShowVideoPage) {
                    switchPageWindow(0);
                    isShowVideoPage = true;
                    isShowPhotoPage = false;
                    isShowMusicPage = false;
                    Log.i(TAG, "switchPageWindow(0)");
                }
            }
        });

        musicTab.setOnFocusChangeListener(new OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                Log.i(TAG, "musicTextView----hasFocus:" + hasFocus
                        + ",isfocused:" + musicTab.isFocused());
                if (hasFocus) {
                    if (!isShowMusicPage) {
                        photoTab.setBackgroundDrawable(photoTabBackgroundDrawable);
                        videoTab.setBackgroundDrawable(videoTabBackgroundDrawable);
                        switchPageWindow(1);
                        isShowMusicPage = true;
                        isShowPhotoPage = false;
                        isShowVideoPage = false;
                        Log.i(TAG, "switchPageWindow(1)");
                    }
                } else {
                    if (isShowMusicPage) {
                        musicTab.setBackgroundResource(R.drawable.tab_selected);
                    }
                }
            }
        });
        musicTab.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (!isShowMusicPage) {
                    switchPageWindow(1);
                    isShowMusicPage = true;
                    isShowPhotoPage = false;
                    isShowVideoPage = false;
                    Log.i(TAG, "switchPageWindow(1)");
                }
            }
        });

        photoTab.setOnFocusChangeListener(new OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                Log.i(TAG, "photoTextView----hasFocus:" + hasFocus
                        + ",isfocused:" + photoTab.isFocused());
                if (hasFocus) {
                    if (!isShowPhotoPage) {
                        videoTab.setBackgroundDrawable(videoTabBackgroundDrawable);
                        musicTab.setBackgroundDrawable(musicTabBackgroundDrawable);
                        switchPageWindow(2);
                        isShowPhotoPage = true;
                        isShowMusicPage = false;
                        isShowVideoPage = false;
                        Log.i(TAG, "switchPageWindow(2)");
                    }
                } else {
                    if (isShowPhotoPage) {
                        photoTab.setBackgroundResource(R.drawable.tab_selected);
                    }
                }
            }
        });
        photoTab.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

                if (!isShowPhotoPage) {
                    switchPageWindow(2);
                    isShowPhotoPage = true;
                    isShowMusicPage = false;
                    isShowVideoPage = false;
                    Log.i(TAG, "switchPageWindow(2)");
                }
            }
        });

        if (!videoTab.isFocused()) {
            if ((tempView = getCurrentFocus()) != null) {
                tempView.clearFocus();
            }
            videoTab.requestFocus();
        }
        if (!isShowVideoPage) {
            switchPageWindow(0);
            isShowVideoPage = true;
            Log.i(TAG, "onCreate----switchPageWindow(0)");
        }
    }

    private Window getPageWindow(int pageID) {
        if (pageIntents == null) {
            pageIntents = new Intent[3];
            pageIntents[0] = new Intent(MediaFileManager.this,
                    com.hybroad.video.VideoBrowser.class);
            pageIntents[1] = new Intent(MediaFileManager.this,
                    com.hybroad.music.MusicBrowser.class);
            pageIntents[2] = new Intent(MediaFileManager.this,
                    com.hybroad.image.ImageBrowser.class);
        }

        if (pageID < 3 && pageID >= 0) {
            return getLocalActivityManager().startActivity(
                    "subPageWindow" + pageID, pageIntents[pageID]);
        } else {
            return getLocalActivityManager().startActivity("subPageWindow" + 0,
                    pageIntents[0]);
        }
    }

    private void switchPageWindow(int pageID) {
        pageContainer.removeAllViews();

        Window pageWindow = null;
        switch (pageID) {
        case 1:
            pageWindow = getPageWindow(1);
            break;

        case 2:
            pageWindow = getPageWindow(2);
            break;

        case 0:
        default:
            pageWindow = getPageWindow(0);
            break;
        }

        pageContainer.addView(pageWindow.getDecorView(),
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
    }

    class setFocusBroadcastReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("SetVideoTabFocused")) {
                videoTab.requestFocus();
            } else if (intent.getAction().equals("SetMusicTabFocused")) {
                musicTab.requestFocus();
            } else if (intent.getAction().equals("SetPhotoTabFocused")) {
                photoTab.requestFocus();
            }
        }
    }

    final StorageEventListener mStorageEventListener = new StorageEventListener() {
        @Override
        public void onStorageStateChanged(String path, String oldState,
                String newState) {
            super.onStorageStateChanged(path, oldState, newState);
            Log.d(TAG, "--onStorageStateChanged--" + newState);
            if (newState.equals("unmounted")) {
                if (DevicesList.s_currentDevice != null
                        && path != null
                        && path.equals(DevicesList.s_currentDevice
                                .getRootPath())) {
                    finish();
                }
            }
        }
    };
}
