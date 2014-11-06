package com.hybroad.image;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Random;

import org.apache.http.conn.ConnectTimeoutException;

import android.R.anim;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.graphics.RectF;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.StrictMode;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;
import android.widget.Toast;

import com.hybroad.dlna.AirPlayService;
import com.hybroad.dlna.DMPMessageTransit;
import com.hybroad.dlna.DMRSharingInformation;
import com.hybroad.dlna.OnCurrentDMSChanged;
import com.hybroad.media.DevicesList;
import com.hybroad.media.R;
import com.hybroad.util.Converter;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.PublicConstants.DialogReslut;
import com.hybroad.util.PublicConstants.IntentStarter;
import com.hybroad.view.gif.GifView;

public class ImagePlayer extends Activity {
    private static final String TAG = "ImagePlayer";
    private static final String UN_MOUNTED = "unmounted";
    private static final int HTTP_CONNECTION_TIMEOUT = 5000;
    private static final int HTTP_CONNECTION_SUCCESS = 200;
    private static final String HTTP_CONNECTION_METHOD = "GET";
    private int mFirstEnterPosition;
    private int mTargetImageShowPosition;
    // add 20131127
    private GifView imageContentView;
    private boolean isGif = false;
    private boolean isGifShow = false;
    private boolean isUnsupportShow = false;
    private final static long MAX_GIF_SIZE = 6291456; // 6 * 1024 * 1024
    private PaintFlagsDrawFilter paintFilter = new PaintFlagsDrawFilter(0,
            Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);

    private View mImageMenuLayout;
    private View mDetailInfoMenuLayout;
    private View mClockwiseRotationMenuLayout;
    private View mAnticlockwiseRotationMenuLayout;
    private View mBackgroundMusicMenuLayout;
    private ImageView mBackgroundMusicMenuIcon;
    private TextView mBackgroundMusicMenuTitle;

    private View mImageDetailInfoLayout;
    private View mImageSlidePlayView;
    private View mImageSlidePauseView;
    private View mNoFavoriteMusicView;
    private TextView mDetailInfoMenuTitleView;
    private TextView mDetailInfoFilenameView;
    private TextView mDetailInfoDateView;
    private TextView mDetailInfoResolutionView;
    private TextView mDetailInfoNumberView;
    private Toast toastPlayStatus;
    private ShowImageReceiver imageMediaActionReceiver;
    private Handler switchImageHandler = null;
    private SwitchImageRunnable switchImageRunnable = null;
    private boolean mStopSwitchImageThreadFlag = false;
    private Thread switchImageTimerThread = null;
    private long switchInterval = 5000;
    private boolean isAutoPlaySlide = true;
    private static final int VISIBLE = 0;
    private static final int INVISIBLE = 4;
    private int currentAnimationNumber = 0;
    private DisplayMetrics displayMetrics;
    private boolean firstEnterShow = true;

    // the background width and height
    private static final int TARGET_PIXELS_WIDTH = 1280;
    private static final int TARGET_PIXELS_HEIGHT = 720;

    private BitmapCacheManager bitmapCache;
    private int currentBitmapDegree = 0;
    private StorageManager mStorageManager = null;

    private ArrayList<String> mBackgroundMusicList = null;
    private int mTargetMusicPosition = 0;
    private boolean misPlayingMusic = false;
    private Context mContext;

    private int mPlayIntentType = IntentStarter.NORMAL_START;
    private String mDMRImagePath;
    private int mDMRDialogResult = DialogReslut.NOTHING;
    private DMPMessageTransit mDMPMessageTransit;
    private DMRMessageReceiver mDMRMessageReceiver;
    // add 20131206 for the airPlay pic play begin
    private boolean mIsAirPlaySlideShow = false;
    // add 20131206 for the airPlay pic play end
    private AirPlayService mAirPlayService = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);

        mContext = this;

        StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder()
                .detectLeakedSqlLiteObjects().detectLeakedClosableObjects()
                .penaltyLog().penaltyDeath().build());
        StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                .detectDiskReads().detectDiskWrites().detectNetwork()
                .penaltyLog().build());

        getWindow().setFormat(android.graphics.PixelFormat.TRANSLUCENT);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        if (displayMetrics == null) {
            displayMetrics = new DisplayMetrics();
        }
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

        if (bitmapCache == null) {
            bitmapCache = BitmapCacheManager.getInstance(TARGET_PIXELS_WIDTH,
                    TARGET_PIXELS_HEIGHT);
        }

        Intent intent = getIntent();
        String starter = intent
                .getStringExtra(PublicConstants.IntentStarter.NORMAL_START_IMAGE_KEY);
        Log.d(TAG, "onCreate(): starter = " + starter);
        if (!TextUtils.isEmpty(starter)
                && starter
                        .equals(PublicConstants.IntentStarter.NORMAL_START_IMAGE_NAME)) {
            mPlayIntentType = IntentStarter.NORMAL_START;
            mFirstEnterPosition = getIntent().getIntExtra(
                    PublicConstants.SELECTED_ITEM_POSITION, 0);
            Log.d(TAG, "onCreate(): mFirstEnterPosition = "
                    + mFirstEnterPosition);
            mTargetImageShowPosition = mFirstEnterPosition;
            setContentView(R.layout.image_player_main);
            initLayoutViewAndData();
            startSwitchImageThread();
        } else {
            mPlayIntentType = IntentStarter.DMR_START;
            mDMRImagePath = intent
                    .getStringExtra(PublicConstants.DMRIntent.KEY_URL);
            Log.d(TAG, "onCreate(): mDMRImagePath :" + mDMRImagePath);
            if (TextUtils.isEmpty(mDMRImagePath)) {
                finish();
            }
            String originalTitle = intent
                    .getStringExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE);
            Log.d(TAG, "onCreate(): originalTitle :" + originalTitle);

            // add 20131206 for the airPlay pic play begin
            mIsAirPlaySlideShow = intent.getBooleanExtra(PublicConstants.DMRIntent.KEY_PIC_SLIDESHOW,
                    false);
            Log.d(TAG, "mIsAirPlaySlideShow = " + mIsAirPlaySlideShow);
            // add 20131206 for the airPlay pic play end
            // popDMRRequestDialogOnCreate();
            ImagePlayList.sCurrentImageList = new ArrayList<ImageFile>();
            ImageFile image = new ImageFile();
            image.filePath = mDMRImagePath;

            if (TextUtils.isEmpty(originalTitle)) {
                int indexStart = mDMRImagePath.lastIndexOf('/') + 1;
                int indexEnd = mDMRImagePath.lastIndexOf('.');
                Log.d(TAG, "onCreate(): indexStart = " + indexStart
                        + ", indexEnd =" + indexEnd);
                if (indexStart >= 0 && indexStart < indexEnd
                        && indexEnd < mDMRImagePath.length()) {
                    image.title = mDMRImagePath.substring(indexStart, indexEnd);
                } else {
                    image.title = mDMRImagePath;
                }
            } else {
                image.title = originalTitle;
            }

            image.setTypeImage();
            ImagePlayList.sCurrentImageList.add(image);

            setContentView(R.layout.image_player_main);
            initLayoutViewAndData();
            final String path = ImagePlayList.sCurrentImageList
                    .get(mTargetImageShowPosition).filePath;
            Log.d(TAG, "path = " + path);
            new AsyncSwitchImageTask().execute(path, false, 0);
            // switchImage();

        }
    }



    @Override
    protected void onNewIntent(Intent intent) {
        Log.i(TAG, "onNewIntent");
        super.onNewIntent(intent);
        setIntent(intent);

        mDMRImagePath = intent
                .getStringExtra(PublicConstants.DMRIntent.KEY_URL);
        if (TextUtils.isEmpty(mDMRImagePath)) {
            return;
        }
        String originalTitle = intent
                .getStringExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE);

        // popDMRRequestDialogOnNewIntent();
        mPlayIntentType = IntentStarter.DMR_START;
        ImagePlayList.sCurrentImageList = null; // don't use clear(), or backup
                                                // is also clear
        ImagePlayList.sCurrentImageList = new ArrayList<ImageFile>();
        ImageFile image = new ImageFile();
        image.filePath = mDMRImagePath;

        if (TextUtils.isEmpty(originalTitle)) {
            int indexStart = mDMRImagePath.lastIndexOf('/') + 1;
            int indexEnd = mDMRImagePath.lastIndexOf('.');
            Log.d(TAG, "onNewIntent: indexStart = " + indexStart
                    + ", indexEnd =" + indexEnd);
            if (indexStart >= 0 && indexStart < indexEnd
                    && indexEnd < mDMRImagePath.length()) {
                image.title = mDMRImagePath.substring(indexStart, indexEnd);
            } else {
                image.title = mDMRImagePath;
            }
        } else {
            image.title = originalTitle;
        }

        image.setTypeImage();
        ImagePlayList.sCurrentImageList.add(image);
        mTargetImageShowPosition = 0;

        setContentView(R.layout.image_player_main);
        initLayoutViewAndData();
        String path = ImagePlayList.sCurrentImageList
                .get(mTargetImageShowPosition).filePath;
        Log.d(TAG, "onCreate(): path = " + path);
        new AsyncSwitchImageTask().execute(path, false, 0);

        // add 20131127 begin
        // switchImage();
    }

    @Override
    protected void onRestart() {
        Log.i(TAG, "onRestart");
        super.onRestart();
    }

    @Override
    protected void onStart() {
        Log.i(TAG, "onStart");
        super.onStart();

        registerReceiver();

        if (null != DevicesList.s_currentDevice
                && DevicesList.s_currentDevice.isLocalDevice()) {
            if (mStorageManager == null) {
                mStorageManager = (StorageManager) ImagePlayer.this
                        .getSystemService(STORAGE_SERVICE);
            }
//            mStorageManager.registerListener(mStorageEventListener);
        } else if (null != DevicesList.s_currentDevice
                && DevicesList.s_currentDevice.isDMSDevice()) {
            if (null == mDMPMessageTransit) {
                mDMPMessageTransit = new DMPMessageTransit(
                        new OnCurrentDMSChanged() {
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

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        isGifShow = false;
        isUnsupportShow = true;
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
        mStopSwitchImageThreadFlag = true;
        if (bitmapCache != null) {
            bitmapCache.clearCache();
        }

        closeBackgroundMusic();

        if (null != mStorageManager) {
//            mStorageManager.unregisterListener(mStorageEventListener);
        }
        if (null != mDMPMessageTransit) {
            unregisterReceiver(mDMPMessageTransit);
        }
        if (null != mMusicMessageReceiver) {
            unregisterReceiver(mMusicMessageReceiver);
        }
        if (null != imageMediaActionReceiver) {
            unregisterReceiver(imageMediaActionReceiver);
        }
        if (null != mDMRMessageReceiver) {
            unregisterReceiver(mDMRMessageReceiver);
        }

        CustomWidget.releaseLoadIcon();

    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy");
        super.onDestroy();

        if (IntentStarter.DMR_START == mPlayIntentType)
            ImagePlayList.sCurrentImageList = ImagePlayList.sCurrentImageListBackup;

        // if (mConnection != null) {
        // unbindService(mConnection);
        // Log.d(TAG, "onDestory(): mAirPlayService = " + mAirPlayService);
        // mAirPlayService = null;
        // }
    }

    private void startSwitchImageThread() {
        if (switchImageHandler == null) {
            switchImageHandler = new Handler();
        }
        if (switchImageRunnable == null) {
            switchImageRunnable = new SwitchImageRunnable(ImagePlayer.this);
        }
        if (switchImageTimerThread == null) {
            switchImageTimerThread = new Thread(new SwitchImageTimerRunnable());
        }
        if (bitmapCache != null
                && ImagePlayList.sCurrentImageList != null
                && mTargetImageShowPosition >= 0
                && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                        .size()) {
            // updateImageView(imageContentView,
            // bitmapCache.getBitmapPackageFromPath(this,
            // ImagePlayList.sCurrentImageList.get(mTargetImageShowPosition).filePath),
            // false, 0);
            final String path = ImagePlayList.sCurrentImageList
                    .get(mTargetImageShowPosition).filePath;
            new AsyncSwitchImageTask().execute(path, false, 0);
            // switchImage();
            mStopSwitchImageThreadFlag = false;
            switchImageTimerThread.start();
        }
    }

    private void initLayoutViewAndData() {
        // imageContentView = (ImageView)
        // this.findViewById(R.id.image_show_content);
        imageContentView = (GifView) this.findViewById(R.id.image_show_content);
        // mSurfaceView = (SurfaceView) findViewById(R.id.surfaceView1);
        // mGifView = (GifView) findViewById(R.id.gifView1);
        // mSurfaceView.setZOrderOnTop(true);
        // mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        // mSurfaceView.getHolder().addCallback(new SurfaceCallback());

        mImageMenuLayout = this.findViewById(R.id.image_menu_layout_content);
        mDetailInfoMenuLayout = this.findViewById(R.id.detail_info_menu_layout);
        mClockwiseRotationMenuLayout = this
                .findViewById(R.id.clockwise_rotation_menu_layout);
        mAnticlockwiseRotationMenuLayout = this
                .findViewById(R.id.anticlockwise_rotation_menu_layout);

        // initialize the background music menu layout
        mBackgroundMusicMenuLayout = this
                .findViewById(R.id.background_music_menu_layout);
        mBackgroundMusicMenuIcon = (ImageView) this
                .findViewById(R.id.background_music_menu_icon);
        mBackgroundMusicMenuTitle = (TextView) this
                .findViewById(R.id.background_music_menu_title);

        // initialize the image detail information layout
        mImageDetailInfoLayout = this
                .findViewById(R.id.image_detail_info_content);
        mDetailInfoMenuTitleView = (TextView) this
                .findViewById(R.id.detail_info_menu_title);
        mDetailInfoFilenameView = (TextView) this
                .findViewById(R.id.detail_info_filename);
        mDetailInfoDateView = (TextView) this
                .findViewById(R.id.detail_info_date);
        mDetailInfoResolutionView = (TextView) this
                .findViewById(R.id.detail_info_resolution);
        mDetailInfoNumberView = (TextView) this
                .findViewById(R.id.detail_info_number);

        // initialize image player slideShow layout
        mImageSlidePlayView = getLayoutInflater().inflate(
                R.layout.image_player_start_slideshow, null);
        mImageSlidePauseView = getLayoutInflater().inflate(
                R.layout.image_player_stop_slideshow, null);

        mNoFavoriteMusicView = getLayoutInflater().inflate(
                R.layout.image_player_no_background_music, null);
    }

    private void registerReceiver() {
        // register music receiver
        IntentFilter musicFilter = new IntentFilter();
        musicFilter
                .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_FINISH);
        musicFilter
                .addAction(PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR);
        registerReceiver(mMusicMessageReceiver, musicFilter);

        imageMediaActionReceiver = new ShowImageReceiver();
        IntentFilter mediaActionFilter = new IntentFilter();
        mediaActionFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        mediaActionFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        mediaActionFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        mediaActionFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        mediaActionFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        mediaActionFilter.addDataScheme("file"); // important!!!
        registerReceiver(imageMediaActionReceiver, mediaActionFilter);

        mDMRMessageReceiver = new DMRMessageReceiver();
        IntentFilter mDMRFilter = new IntentFilter();
        mDMRFilter.addAction(PublicConstants.DMRIntent.ACTION_DMR_EVENT);
        registerReceiver(mDMRMessageReceiver, mDMRFilter);
    }

    final StorageEventListener mStorageEventListener = new StorageEventListener() {
        @Override
        public void onStorageStateChanged(String path, String oldState,
                String newState) {
            super.onStorageStateChanged(path, oldState, newState);
            if (newState.equals(UN_MOUNTED)) {
                if (DevicesList.s_currentDevice != null
                        && path != null
                        && path.equals(DevicesList.s_currentDevice
                                .getRootPath())) {
                    finish();
                }
            }
        }
    };

    // modifty 20131127
    private void updateGifView(GifView gifView, GifPackage gifPackage) {
        Log.d(TAG, "onPostExecute: updateGifView()");
        if (gifView == null) {
            return;
        }
        gifView.setDrawThreadRunFlag(false); //
        // ------------------test-----------------------------

        if (null == gifPackage || null == gifPackage.content) {
            gifView.setScaleType(ScaleType.CENTER_INSIDE);

            if (null == ImagePlayList.sCurrentImageList
                    || mTargetImageShowPosition < 0
                    || mTargetImageShowPosition >= ImagePlayList.sCurrentImageList
                            .size()
                    || null == ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition)
                    || null == ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition).filePath) {
                gifView.setImageResource(R.drawable.hint_data_exception);
                updateImageDetailInfo(0, 0);
                return;
            }

            if (ImagePlayList.sCurrentImageList.get(mTargetImageShowPosition).filePath
                    .startsWith("http://")) {
                gifView.setImageResource(R.drawable.hint_network_anomaly);
            } else {
                gifView.setImageResource(R.drawable.hint_data_exception);
            }
            updateImageDetailInfo(0, 0);
            return;
        }

        gifView.setScaleType(ScaleType.CENTER_INSIDE);
        if (!isAutoPlaySlide) {
            Log.d(TAG, "updateGifView: isAutoPlaySlide: " + isAutoPlaySlide);
            gifView.setGifImageType(GifView.GifImageType.COVER);
            gifView.setGifImage(gifPackage.content, false);
            if (firstEnterShow) {
                firstEnterShow = false;
            } else {
                // mGifView.startAnimation(AnimationUtils.loadAnimation(this,
                // getAnimationID()));
                gifView.startAnimation(AnimationUtils.loadAnimation(this,
                        anim.fade_in));
            }
        } else {
            Log.d(TAG, "updateGifView: isAutoPlaySlide: " + isAutoPlaySlide);
            gifView.setGifImageType(GifView.GifImageType.SYNC_DECODER);
            gifView.setDrawThreadRunFlag(true); //
            // ------------------test-----------------------------
            gifView.setGifImage(gifPackage.content, true);
        }

        updateImageDetailInfo(gifPackage.width, gifPackage.height);
    }

    private void updateImageView(ImageView imageView,
            BitmapPackage bitmapPackage, boolean rotateEnable, int rotateDgree) {
        Log.d(TAG, "updateImageView():");
        if (imageView == null) {
            Log.e(TAG, "imageView is null");
            return;
        }

        Bitmap bitmapTemp = null;
        if (bitmapPackage == null
                || (bitmapTemp = bitmapPackage.getBitmap()) == null) {
            imageView.setScaleType(ScaleType.CENTER_INSIDE);

            if (null == ImagePlayList.sCurrentImageList
                    || mTargetImageShowPosition < 0
                    || mTargetImageShowPosition >= ImagePlayList.sCurrentImageList
                            .size()
                    || null == ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition)
                    || null == ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition).filePath) {
                imageView.setImageResource(R.drawable.hint_data_exception);
                updateImageDetailInfo(0, 0);
                return;
            }

            if (ImagePlayList.sCurrentImageList.get(mTargetImageShowPosition).filePath
                    .startsWith("http://")) {
                imageView.setImageResource(R.drawable.hint_network_anomaly);
            } else {
                imageView.setImageResource(R.drawable.hint_data_exception);
            }
            updateImageDetailInfo(0, 0);
            return;
        }

        if (rotateEnable) {
            int originalWidth = bitmapPackage.getOriginalWidth();
            int originalHeight = bitmapPackage.getOriginalHeight();
            Log.d(TAG, "originalWidth = " + originalWidth
                    + ", originalHeight = " + originalHeight);
            Bitmap bitmapRotate = ImageUtils.rotate(bitmapTemp, rotateDgree);
            if (bitmapTemp != bitmapRotate) { // rotate success
                bitmapTemp = bitmapRotate;
                bitmapPackage.setBitmap(bitmapTemp);
                bitmapPackage.setOriginalWidth(originalHeight);
                bitmapPackage.setOriginalHeight(originalWidth);
                bitmapPackage.setDegreeDelta(rotateDgree);
                currentBitmapDegree = bitmapPackage.getDegree();
                Log.d(TAG, "mCurrentBitmap current degree: "
                        + currentBitmapDegree);
            } else {
                Log.d(TAG, "rotate failure!");
                return;
            }
        }

        if (bitmapPackage.getSampleValue() > 1) {
            Matrix matrix = getCenterScaledMatrix(bitmapTemp, imageView,
                    TARGET_PIXELS_WIDTH, TARGET_PIXELS_HEIGHT,
                    displayMetrics.widthPixels, displayMetrics.heightPixels);
            if (matrix != null) {
                imageView.setScaleType(ScaleType.MATRIX);
                imageView.setImageMatrix(matrix);
            } else {
                imageView.setScaleType(ScaleType.CENTER_INSIDE);
            }
        } else {
            imageView.setScaleType(ScaleType.CENTER_INSIDE);
        }

        try {
            Log.d(TAG, "setImageBitmap: bitmapTemp = " + bitmapTemp);
            imageView.setImageBitmap(bitmapTemp);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }

        // the sdcard sildeShow play with animations 
        if (isAutoPlaySlide) {
            if (firstEnterShow) {
                firstEnterShow = false;
            } else {
//                imageView.startAnimation(AnimationUtils.loadAnimation(this,
//                        getAnimationID()));
                 imageView.startAnimation(AnimationUtils.loadAnimation(this,
                 anim.fade_in));
            }
        }
        if (mIsAirPlaySlideShow) {
            Log.d(TAG, "airPlay SlideShow play");
//            imageView.startAnimation(AnimationUtils.loadAnimation(this,
//                    anim.fade_in));
            if (firstEnterShow) {
                firstEnterShow = false;
            } else {
                imageView.startAnimation(AnimationUtils.loadAnimation(this,
                        getAnimationID()));
            }
        }

        // the airplay sildeShow play with animations
        
        updateImageDetailInfo(bitmapPackage.getOriginalWidth(),
                bitmapPackage.getOriginalHeight());
    }

    private Matrix getCenterScaledMatrix(Bitmap bitmap, ImageView imageView,
            int targetPixelsWidth, int targetPixelsHeight, int screenWidth,
            int screenHeight) {
        if (bitmap == null || imageView == null) {
            return null;
        }

        float scale;
        Matrix matrix = new Matrix();
        Matrix matrixTemp = new Matrix();

        // set scale
        float scaleWidth = (float) targetPixelsWidth / bitmap.getWidth();
        float scaleHeight = (float) targetPixelsHeight / bitmap.getHeight();
        if (scaleWidth > scaleHeight) {
            scale = scaleHeight;
        } else {
            scale = scaleWidth;
        }
        matrix.setScale(scale, scale);

        // set postXY
        matrixTemp.set(matrix);
        RectF rect = new RectF(0, 0, bitmap.getWidth(), bitmap.getHeight());
        matrixTemp.mapRect(rect);
        float rectHeight = rect.height();
        float rectWidth = rect.width();
        float deltaX = 0, deltaY = 0;
        if (rectHeight < screenHeight) {
            deltaY = (screenHeight - rectHeight) / 2 - rect.top;
        } else if (rect.top > 0) {
            deltaY = -rect.top;
        } else if (rect.bottom < screenHeight) {
            deltaY = imageView.getHeight() - rect.bottom;
        }
        if (rectWidth < screenWidth) {
            deltaX = (screenWidth - rectWidth) / 2 - rect.left;
        } else if (rect.left > 0) {
            deltaX = -rect.left;
        } else if (rect.right < screenWidth) {
            deltaX = screenWidth - rect.right;
        }
        matrix.postTranslate(deltaX, deltaY);

        return matrix;
    }

    private int getAnimationID() {
        int currentAnimationID = 0;

        currentAnimationNumber = getShufflerNumber(3, currentAnimationNumber);
        switch (currentAnimationNumber) {
        case 0:
            currentAnimationID = anim.fade_in;
            break;
        case 1:
            currentAnimationID = R.anim.push_right_in;
            break;
        case 2:
            currentAnimationID = R.anim.push_left_in;
            break;

        default:
            break;
        }

        return currentAnimationID;
    }

    private void updateImageDetailInfo(int width, int height) {
        Log.d(TAG, "updateImageDetailInfo ");
        if (ImagePlayList.sCurrentImageList == null
                || mTargetImageShowPosition < 0
                || mTargetImageShowPosition >= ImagePlayList.sCurrentImageList
                        .size()) {
            return;
        }
        mDetailInfoFilenameView.setText(ImagePlayList.sCurrentImageList
                .get(mTargetImageShowPosition).title);
        mDetailInfoDateView.setText(ImagePlayList.sCurrentImageList
                .get(mTargetImageShowPosition).dateTaken);
        mDetailInfoResolutionView.setText(width + "x" + height);
        mDetailInfoNumberView.setText(mTargetImageShowPosition + 1 + "/"
                + ImagePlayList.sCurrentImageList.size());
    }

    private int getShufflerNumber(int interval, int previous) {
        Random mRandom = new Random();
        int ret;
        do {
            ret = mRandom.nextInt(interval);
            // Returns a pseudo-random uniformly distributed int in the
            // half-open range [0, interval).
        } while (ret == previous && interval > 1);
        Log.d(TAG, "getShufflerNumber random ret = " + ret);
        return ret;
    }

    class ShowImageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {

            } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {

            } else if (action.equals(Intent.ACTION_MEDIA_SCANNER_FINISHED)) {

            } else if (action.equals(Intent.ACTION_MEDIA_EJECT)) {

            }
        }

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_DPAD_LEFT:
            if (bitmapCache != null && ImagePlayList.sCurrentImageList != null
                    && ImagePlayList.sCurrentImageList.size() > 0) {
                if (mImageMenuLayout.isShown()) {
                    if (mBackgroundMusicMenuLayout.isFocused()) {
                        mAnticlockwiseRotationMenuLayout.requestFocus();
                    } else if (mAnticlockwiseRotationMenuLayout.isFocused()) {
                        mClockwiseRotationMenuLayout.requestFocus();
                    } else if (mClockwiseRotationMenuLayout.isFocused()) {
                        mDetailInfoMenuLayout.requestFocus();
                    }
                    return true;
                }
                stopPlaySlide();
                if (currentBitmapDegree != 0) { // clear bmpCache which degree
                                                // is not 0 before switch bmp
                    if (mTargetImageShowPosition >= 0
                            && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                    .size()) {
                        bitmapCache
                                .deleteCacheByKey(ImagePlayList.sCurrentImageList
                                        .get(mTargetImageShowPosition).filePath);
                    }
                }
                if (mTargetImageShowPosition > 0) {
                    mTargetImageShowPosition--;
                } else {
                    mTargetImageShowPosition = ImagePlayList.sCurrentImageList
                            .size() - 1;
                }
                if (mTargetImageShowPosition >= 0
                        && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                .size()) {
                    final String path = ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition).filePath;
                    new AsyncSwitchImageTask().execute(path, false, 0);
                    // switchImage();
                }
            }
            return true;

        case KeyEvent.KEYCODE_DPAD_RIGHT:
            if (bitmapCache != null && ImagePlayList.sCurrentImageList != null
                    && ImagePlayList.sCurrentImageList.size() > 0) {
                if (mImageMenuLayout.isShown()) {
                    if (mDetailInfoMenuLayout.isFocused()) {
                        mClockwiseRotationMenuLayout.requestFocus();
                    } else if (mClockwiseRotationMenuLayout.isFocused()) {
                        mAnticlockwiseRotationMenuLayout.requestFocus();
                    } else if (mAnticlockwiseRotationMenuLayout.isFocused()) {
                        mBackgroundMusicMenuLayout.requestFocus();
                    }
                    return true;
                }
                stopPlaySlide();
                if (currentBitmapDegree != 0) { // clear bmpCache which degree
                                                // is not 0 before switch bmp
                    if (mTargetImageShowPosition >= 0
                            && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                    .size()) {
                        bitmapCache
                                .deleteCacheByKey(ImagePlayList.sCurrentImageList
                                        .get(mTargetImageShowPosition).filePath);
                    }
                }
                if (mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                        .size() - 1) {
                    mTargetImageShowPosition++;
                } else {
                    mTargetImageShowPosition = 0;
                }
                if (mTargetImageShowPosition >= 0
                        && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                .size()) {
                    final String path = ImagePlayList.sCurrentImageList
                            .get(mTargetImageShowPosition).filePath;
                    new AsyncSwitchImageTask().execute(path, false, 0);
                    // switchImage();
                }
            }
            return true;

        case KeyEvent.KEYCODE_DPAD_UP:
            if (mImageMenuLayout.isShown()) {
                return true;
            }
            if (bitmapCache != null
                    && ImagePlayList.sCurrentImageList != null
                    && mTargetImageShowPosition >= 0
                    && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                            .size()) {
                final String path = ImagePlayList.sCurrentImageList
                        .get(mTargetImageShowPosition).filePath;
                if (null != path && !path.toLowerCase().endsWith(".gif")) {
                    new AsyncSwitchImageTask().execute(path, true, 90);
                    // switchImage();
                }
            }
            return true;

        case KeyEvent.KEYCODE_DPAD_DOWN:
            if (mImageMenuLayout.isShown()) {
                return true;
            }
            if (bitmapCache != null
                    && ImagePlayList.sCurrentImageList != null
                    && mTargetImageShowPosition >= 0
                    && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                            .size()) {
                final String path = ImagePlayList.sCurrentImageList
                        .get(mTargetImageShowPosition).filePath;
                if (null != path && !path.toLowerCase().endsWith(".gif")) {
                    new AsyncSwitchImageTask().execute(path, true, -90);
                    // switchImage();
                }
            }
            return true;

        case KeyEvent.KEYCODE_DPAD_CENTER:
            if (mImageMenuLayout.isShown()) {
                if (mDetailInfoMenuLayout.isFocused()) {
                    mImageMenuLayout.setVisibility(INVISIBLE);
                    if (mImageDetailInfoLayout.isShown()) {
                        mImageDetailInfoLayout.setVisibility(INVISIBLE);
                        mDetailInfoMenuTitleView
                                .setText(R.string.show_detail_info);
                    } else {
                        mImageDetailInfoLayout.setVisibility(VISIBLE);
                        mDetailInfoMenuTitleView
                                .setText(R.string.hide_detail_info);
                    }
                } else if (mClockwiseRotationMenuLayout.isFocused()) {
                    if (bitmapCache != null
                            && ImagePlayList.sCurrentImageList != null
                            && mTargetImageShowPosition >= 0
                            && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                    .size()) {
                        final String path = ImagePlayList.sCurrentImageList
                                .get(mTargetImageShowPosition).filePath;
                        if (null != path
                                && !path.toLowerCase().endsWith(".gif")) {
                            new AsyncSwitchImageTask().execute(path, true, 90);
                            // switchImage();
                        }
                    }
                } else if (mAnticlockwiseRotationMenuLayout.isFocused()) {
                    if (bitmapCache != null
                            && ImagePlayList.sCurrentImageList != null
                            && mTargetImageShowPosition >= 0
                            && mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                                    .size()) {
                        final String path = ImagePlayList.sCurrentImageList
                                .get(mTargetImageShowPosition).filePath;
                        if (null != path
                                && !path.toLowerCase().endsWith(".gif")) {
                            new AsyncSwitchImageTask().execute(path, true, -90);
                            // switchImage();
                        }
                    }
                } else if (mBackgroundMusicMenuLayout.isFocused()) {
                    mImageMenuLayout.setVisibility(INVISIBLE);
                    if (misPlayingMusic) {
                        closeBackgroundMusic();
                        mBackgroundMusicMenuIcon
                                .setImageResource(R.drawable.open_background_music);
                        mBackgroundMusicMenuTitle
                                .setText(R.string.open_background_music);
                    } else {
                        if (openBackgroundMusic()) {
                            mBackgroundMusicMenuIcon
                                    .setImageResource(R.drawable.close_background_music);
                            mBackgroundMusicMenuTitle
                                    .setText(R.string.close_background_music);
                        }
                    }
                }
                return true;
            }
            if (isAutoPlaySlide) {
                stopPlaySlide();
            } else {
                startPlaySlide();
            }
            return true;

        case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
            if (mImageMenuLayout.isShown()) {
                return true;
            }
            if (isAutoPlaySlide) {
                stopPlaySlide();
            } else {
                startPlaySlide();
            }
            return true;

        case KeyEvent.KEYCODE_MENU:
            if (mImageMenuLayout.isShown()) {
                mImageMenuLayout.setVisibility(INVISIBLE);
            } else {
                mImageMenuLayout.setVisibility(VISIBLE);
                mDetailInfoMenuLayout.requestFocus();
            }
            return true;

        case KeyEvent.KEYCODE_BACK:
            if (mImageMenuLayout.isShown()) {
                mImageMenuLayout.setVisibility(INVISIBLE);
                return true;
            } else {
                return super.onKeyDown(keyCode, event);
            }

        case KeyEvent.KEYCODE_MEDIA_STOP:
            if (mImageMenuLayout.isShown()) {
                return true;
            }
            finish();

        default:
            break;
        }

        return super.onKeyDown(keyCode, event);
    }

    // start the play slideShow
    private boolean startPlaySlide() {
        if (!isAutoPlaySlide) {
            if (toastPlayStatus == null) {
                toastPlayStatus = new Toast(this);
                toastPlayStatus.setGravity(Gravity.TOP, 0, 10);
                toastPlayStatus.setDuration(Toast.LENGTH_SHORT);
            }
            toastPlayStatus.setView(mImageSlidePlayView);
            isAutoPlaySlide = true;
            toastPlayStatus.show();
            return true;
        }
        return false;
    }

    // stop the play slideShow
    private boolean stopPlaySlide() {
        if (isAutoPlaySlide) {
            if (toastPlayStatus == null) {
                toastPlayStatus = new Toast(this);
                toastPlayStatus.setGravity(Gravity.TOP, 0, 10);
                toastPlayStatus.setDuration(Toast.LENGTH_SHORT);
            }
            toastPlayStatus.setView(mImageSlidePauseView);
            isAutoPlaySlide = false;
            toastPlayStatus.show();
            return true;
        }
        return false;
    }

    private void noFavoriteMusic() {
        Toast toast = new Toast(this);
        toast.setGravity(Gravity.BOTTOM, 0, 10);
        toast.setDuration(Toast.LENGTH_SHORT);
        toast.setView(mNoFavoriteMusicView);
        toast.show();
    }

    class SwitchImageRunnable implements Runnable {
        private Context mContext;

        public SwitchImageRunnable(Context context) {
            mContext = context;
        }

        @Override
        public void run() {
            if (ImagePlayList.sCurrentImageList == null
                    || ImagePlayList.sCurrentImageList.size() <= 1
                    || bitmapCache == null || mTargetImageShowPosition < 0) {
                return;
            }
            if (mTargetImageShowPosition < ImagePlayList.sCurrentImageList
                    .size() - 1) {
                
                mTargetImageShowPosition++;
                Log.d(TAG, "SwitchImageRunnable: mTargetImageShowPosition = "
                        + mTargetImageShowPosition);
            } else {
                mTargetImageShowPosition = 0;
                mStopSwitchImageThreadFlag = true;
                Log.d(TAG, "stop the slideshow");
            }
            final String path = ImagePlayList.sCurrentImageList
                    .get(mTargetImageShowPosition).filePath;
            new AsyncSwitchImageTask().execute(path, false, 0);
            // switchImage();
        }
    }

    // a timer runnable that can play the next image automatic
    class SwitchImageTimerRunnable implements Runnable {
        private long startSwitchImageTime;

        @Override
        public void run() {

            try {
                Thread.currentThread();
                startSwitchImageTime = System.currentTimeMillis();
                while (!mStopSwitchImageThreadFlag) {
                    Thread.sleep(500);
                    // when flag turn true, stop slideShow play
                    if (mStopSwitchImageThreadFlag) {
                        return;
                    }
                    if (isAutoPlaySlide) {
                        if (System.currentTimeMillis() - startSwitchImageTime >= switchInterval) {

                            switchImageHandler.post(switchImageRunnable);
                            startSwitchImageTime = System.currentTimeMillis();
                        }
                    } else {
                        startSwitchImageTime = System.currentTimeMillis();
                    }
                }

            } catch (InterruptedException e) {
                Log.e(TAG, "InterruptedException", e);
            }
            switchImageHandler.removeCallbacks(switchImageRunnable);
        }

    }

    private boolean openBackgroundMusic() {
        if (null == mBackgroundMusicList) {
            File directory = new File(
                    PublicConstants.MusicCommon.MUSIC_FAVORITE_PATH);
            File[] files = null;
            if (directory.exists() && directory.isDirectory())
                files = directory.listFiles();
            if (null == files || 0 == files.length) {
                noFavoriteMusic();
                return false;
            }

            mBackgroundMusicList = new ArrayList<String>();
            for (File file : files) {
                if (file.isFile()) {
                    String musicPath = file.getAbsolutePath();
                    mBackgroundMusicList.add(musicPath);
                }
            }
        }
        startMusicPlayService();
        return true;
    }

    private void startMusicPlayService() {
        if (null == mBackgroundMusicList || mBackgroundMusicList.size() == 0)
            return;
        if (mTargetMusicPosition >= mBackgroundMusicList.size()
                || mTargetMusicPosition < 0)
            mTargetMusicPosition = 0;
        String audioFilePath = mBackgroundMusicList.get(mTargetMusicPosition);
        if (TextUtils.isEmpty(audioFilePath))
            return;
        Intent serviceIntent = new Intent(
                PublicConstants.MusicCommon.MUSIC_PLAY_SERVICE);
        serviceIntent.putExtra(
                PublicConstants.MusicCommon.INTENT_KEY_MUSIC_FILE_PATH,
                audioFilePath);
        startService(serviceIntent);
        misPlayingMusic = true;
    }

    private void closeBackgroundMusic() {
        stopService(new Intent(PublicConstants.MusicCommon.MUSIC_PLAY_SERVICE));
        misPlayingMusic = false;
    }

    private BroadcastReceiver mMusicMessageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (PublicConstants.MusicCommon.INTENT_ACTION_PLAY_FINISH
                    .equals(action)
                    || PublicConstants.MusicCommon.INTENT_ACTION_PLAY_ERROR
                            .equals(action)) {
                if (null != mBackgroundMusicList) {
                    if (mTargetMusicPosition < mBackgroundMusicList.size() - 1
                            && mTargetMusicPosition >= 0)
                        mTargetMusicPosition++;
                    else
                        mTargetMusicPosition = 0;

                    startMusicPlayService();
                }
            }
        }
    };

    @SuppressWarnings("rawtypes")
    private class AsyncSwitchImageTask extends AsyncTask {
        private boolean mRotateEnable = false;
        private int mRotateDgree = 0;
        private boolean isGif = false;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            Log.d(TAG, "CustonWidget show the LoadIcon");
            if (!mIsAirPlaySlideShow) {
                CustomWidget.showLoadIcon(mContext);
            }
        }

        @Override
        protected Object doInBackground(Object... params) {
            Log.d("----AsyncSwitchImageTask----", "doInBackground");
            String path = (String) params[0];
            Log.d(TAG, "AsyncSwitchImageTask doInBackground: path = " + path);
            if (TextUtils.isEmpty(path)) {
                return null;
            } else if (path.toLowerCase().endsWith(".gif")) {
                this.isGif = true;
            } else {
                this.isGif = false;
            }
            // isGif = false; // --------------Provisional
            // measures---------------
            Log.d(TAG, "doInBackground: isGif = " + isGif);
            if (isGif) {
                return getGifImageStream(ImagePlayer.this, path);
            } else {
                this.mRotateEnable = (Boolean) params[1];
                this.mRotateDgree = (Integer) params[2];
                Log.d(TAG,
                        "AsyncSwitchImageTask doInBackground: this.mRotateEnable "
                                + this.mRotateEnable + ", this.mRotateDgree = "
                                + this.mRotateDgree);
                return bitmapCache.getBitmapPackageFromPath(ImagePlayer.this,
                        path);
            }
        }

        @Override
        protected void onPostExecute(Object result) {
            Log.d("----AsyncSwitchImageTask----", "onPostExecute");
            CustomWidget.hideLoadIcon();
            Log.d(TAG, "CustonWidget hide the LoadIcon");
            Log.d(TAG, "onPostExecute: isGif = " + isGif);
            if (isGif) {
                Log.d(TAG, "onPostExecute updateGifView");
                updateGifView(imageContentView, (GifPackage) result);
            } else {
                Log.d(TAG, "onPostExecute updateImageView");
                updateImageView(imageContentView, (BitmapPackage) result,
                        this.mRotateEnable, this.mRotateDgree);
            }
        }

    }

    @SuppressWarnings("resource")
    private GifPackage getGifImageStream(Context context, String filePath) {
        Log.d(TAG, "getGifImageStream() ");
        if (TextUtils.isEmpty(filePath) || context == null) {
            return null;
        }
        String transformedPath = Converter.convertSpecialCharacters(filePath);
        Log.d(TAG, "getGifImageStream() transformedPath = " + transformedPath);
        ContentResolver cr = context.getContentResolver();
        InputStream inputStream = null;
        BitmapFactory.Options options = new BitmapFactory.Options();
        HttpURLConnection httpURLConnection = null;
        URL url = null;
        GifPackage gifPackage = null;

        try {
            options.inSampleSize = 1;
            options.inJustDecodeBounds = true;
            if (transformedPath.startsWith("http://")) {
                Log.d(TAG,
                        "getGifImageStream() transformedPath startWith http://");
                url = new URL(transformedPath);
                httpURLConnection = (HttpURLConnection) url.openConnection();
                if (httpURLConnection != null) {
                    httpURLConnection
                            .setConnectTimeout(HTTP_CONNECTION_TIMEOUT);
                    httpURLConnection.setReadTimeout(HTTP_CONNECTION_TIMEOUT);
                    httpURLConnection.setRequestMethod(HTTP_CONNECTION_METHOD);
                    int responseCode;
                    if (HTTP_CONNECTION_SUCCESS == (responseCode = httpURLConnection
                            .getResponseCode())) {
                        inputStream = httpURLConnection.getInputStream();
                    } else {
                        Log.e(TAG, "connect failure! responseCode: "
                                + responseCode);
                        if (httpURLConnection != null) {
                            httpURLConnection.disconnect();
                            httpURLConnection = null;
                        }
                        return null;
                    }
                }
            } else if (transformedPath.startsWith("/")) {
                Log.d(TAG, "getGifImageStream() transformedPath startWith /");
                inputStream = cr.openInputStream(Uri.parse("file://"
                        + transformedPath));
            }

            if (inputStream == null) {
                Log.d(TAG, "inputStream = null");
                if (httpURLConnection != null) {
                    httpURLConnection.disconnect();
                    httpURLConnection = null;
                }
                return null;
            }

            Bitmap bitmap = BitmapFactory.decodeStream(inputStream, null,
                    options);
            Log.d(TAG, "getGifImageStream() transformedPath bitmap = " + bitmap);
            Log.d(TAG, "options.outWidth = " + options.outWidth
                    + ", options.outHeight = " + options.outHeight);
            if (inputStream != null) {
                inputStream.close();
            }
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
            }
            if (options.mCancel || options.outWidth == -1
                    || options.outHeight == -1) {
                Log.d(TAG,
                        "options.mCancel || options.outWidth == -1 || options.outHeight == -1");
                return null;
            }
            gifPackage = new GifPackage();
            gifPackage.width = options.outWidth;
            gifPackage.height = options.outHeight;
            Log.d(TAG, "gifPackage.width = " + gifPackage.width
                    + ", gifPackage.height = " + gifPackage.height);

            if (transformedPath.startsWith("http://")) {
                Log.d(TAG,
                        "getGifImageStream() first decode transformedPath startWith http://");
                httpURLConnection = (HttpURLConnection) url.openConnection();
                if (httpURLConnection != null) {
                    httpURLConnection
                            .setConnectTimeout(HTTP_CONNECTION_TIMEOUT);
                    httpURLConnection.setReadTimeout(HTTP_CONNECTION_TIMEOUT);
                    httpURLConnection.setRequestMethod(HTTP_CONNECTION_METHOD);
                    int responseCode;
                    if (HTTP_CONNECTION_SUCCESS == (responseCode = httpURLConnection
                            .getResponseCode())) {
                        inputStream = httpURLConnection.getInputStream();
                        Log.d(TAG,
                                "getGifImageStream() first decode inputStream= "
                                        + inputStream);
                    } else {
                        Log.e(TAG, "connect failure! responseCode: "
                                + responseCode);
                        if (httpURLConnection != null) {
                            httpURLConnection.disconnect();
                            httpURLConnection = null;
                        }
                        return null;
                    }
                }
            } else if (transformedPath.startsWith("/")) {
                Log.d(TAG,
                        "second decode getGifImageStream() transformedPath startWith /");
                inputStream = cr.openInputStream(Uri.parse("file://"
                        + transformedPath));
                Log.d(TAG, "second decode getGifImageStream(): inputStream= "
                        + inputStream);
            }

            if (inputStream == null) {
                Log.d(TAG, "inputStream = null");
                if (httpURLConnection != null) {
                    httpURLConnection.disconnect();
                    httpURLConnection = null;
                }
                return null;
            }
            gifPackage.content = Converter.InputStreamToByte(inputStream);
            Log.d(TAG,
                    "second decode getGifImageStream(): gifPackage.content= "
                            + gifPackage.content);
            inputStream.close();
            inputStream = null;
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
            }
        } catch (ConnectTimeoutException e) {
            Log.e(TAG, "ConnectTimeoutException", e);
            return null;
        } catch (MalformedURLException e) {
            Log.e(TAG, "MalformedURLException", e);
            return null;
        } catch (FileNotFoundException e) {
            Log.e(TAG, "FileNotFoundException", e);
            return null;
        } catch (IOException e) {
            Log.e(TAG, "IOException", e);
            return null;
        } catch (Exception e) {
            Log.e(TAG, "Other Exception", e);
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
            }
        }

        return gifPackage;
    }

    private class GifPackage {
        public byte content[];
        public int width;
        public int height;
    }

    private class DMRMessageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (PublicConstants.DMRIntent.ACTION_DMR_EVENT.equals(action)) {
                int cmd = intent.getIntExtra(
                        PublicConstants.DMRIntent.KEY_PLAY_CMD, -1);
                switch (cmd) {
                case PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP:
                    DMRSharingInformation.sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_STOPPED;

                    finish();

                default:
                    break;
                }
            }
        }

    }

    private void popDMRRequestDialogOnCreate() {
        Dialog playDMRDialog = new AlertDialog.Builder(this)
                .setIcon(android.R.drawable.ic_dialog_info)
                .setTitle(getString(R.string.dmr_play_request))
                .setMessage(
                        getString(R.string.dmr_play_request_content)
                                + mDMRImagePath)
                .setPositiveButton(getString(R.string.confirm),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int arg1) {
                                ImagePlayList.sCurrentImageList = new ArrayList<ImageFile>();
                                ImageFile image = new ImageFile();
                                image.filePath = mDMRImagePath;
                                int indexStart = mDMRImagePath.lastIndexOf('/') + 1;
                                int indexEnd = mDMRImagePath.lastIndexOf('.');
                                if (indexStart >= 0 && indexStart < indexEnd
                                        && indexEnd < mDMRImagePath.length()) {
                                    image.title = mDMRImagePath.substring(
                                            indexStart, indexEnd);
                                } else {
                                    image.title = mDMRImagePath;
                                }
                                image.setTypeImage();
                                ImagePlayList.sCurrentImageList.add(image);

                                mDMRDialogResult = DialogReslut.CONFIRM;
                                dialog.dismiss();

                                setContentView(R.layout.image_player_main);
                                initLayoutViewAndData();
                                final String path = ImagePlayList.sCurrentImageList
                                        .get(mTargetImageShowPosition).filePath;
                                new AsyncSwitchImageTask().execute(path, false,
                                        0);
                                // switchImage();
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
        playDMRDialog
                .setOnDismissListener(new DialogInterface.OnDismissListener() {
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
                        getString(R.string.dmr_play_request_content)
                                + mDMRImagePath)
                .setPositiveButton(getString(R.string.confirm),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int arg1) {
                                mPlayIntentType = IntentStarter.DMR_START;
                                ImagePlayList.sCurrentImageList = null; // don't
                                                                        // use
                                                                        // clear(),
                                                                        // or
                                                                        // backup
                                                                        // is
                                                                        // also
                                                                        // clear
                                ImagePlayList.sCurrentImageList = new ArrayList<ImageFile>();
                                ImageFile image = new ImageFile();
                                image.filePath = mDMRImagePath;
                                int indexStart = mDMRImagePath.lastIndexOf('/') + 1;
                                int indexEnd = mDMRImagePath.lastIndexOf('.');
                                if (indexStart >= 0 && indexStart < indexEnd
                                        && indexEnd < mDMRImagePath.length()) {
                                    image.title = mDMRImagePath.substring(
                                            indexStart, indexEnd);
                                } else {
                                    image.title = mDMRImagePath;
                                }
                                image.setTypeImage();
                                ImagePlayList.sCurrentImageList.add(image);
                                mTargetImageShowPosition = 0;

                                // used to continuous repeated requests pending
                                // case
                                mDMRDialogResult = DialogReslut.CONFIRM;
                                dialog.dismiss();

                                setContentView(R.layout.image_player_main);
                                initLayoutViewAndData();
                                final String path = ImagePlayList.sCurrentImageList
                                        .get(mTargetImageShowPosition).filePath;
                                new AsyncSwitchImageTask().execute(path, false,
                                        0);
                                // switchImage();
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

}
