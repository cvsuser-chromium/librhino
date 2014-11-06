package com.hybroad.video;

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

import java.io.IOException;
import java.util.Map;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
//import android.os.DisplayManager;
import android.os.Parcel;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.MediaController;

import com.hybroad.util.Converter;
import android.hardware.display.DisplayManager;
import com.hybroad.util.StatusBarOperator;

/**
 * Displays a video file. The VideoView class can load images from various
 * sources (such as resources or content providers), takes care of computing its
 * measurement from the video so that it can be used in any layout manager, and
 * provides various display options such as scaling and tinting.
 */
@SuppressLint("Recycle")
public class VideoView extends SurfaceView implements MediaController.MediaPlayerControl {
    private String TAG = "VideoView";

    //
    private Context mContext;

    // settable by the client
    private Uri mUri;
    private int mDuration;
    // add 20131115 liumeidong begin
    private Map<String, String> mHeaders;
    // add 20131115 liumeidong end

    // All the stuff we need for playing and showing a video
    private SurfaceHolder mSurfaceHolder = null;
    public MediaPlayer mMediaPlayer = null;
    private boolean mIsPrepared;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private MediaPlayer.OnCompletionListener mOnCompletionListener;

    private MediaPlayer.OnBufferingUpdateListener onBufferingUpdateListener;
//    private MediaPlayer.OnFastBackwordCompleteListener mOnFastBackwordCompleteListener;
    private MediaPlayer.OnPreparedListener mOnPreparedListener;
    private int mCurrentBufferPercentage;
    private MediaPlayer.OnErrorListener mOnErrorListener;
    private MediaPlayer.OnInfoListener mOn3DModeReceivedListener;
    private MediaPlayer.OnInfoListener mOnInfoListener = null;
    private boolean mStartWhenPrepared;
    private int mSeekWhenPrepared;
    private DisplayManager display_man = null;
    private static final int CMD_STOP_FASTPLAY = 199;
    private MediaController mMediaController;

    public int getVideoWidth() {
        return mVideoWidth;
    }

    public int getVideoHeight() {
        return mVideoHeight;
    }

    /**
     * Set Width and Height of the Video
     * 
     * @param width
     * @param height
     */
    public void setVideoScale(int width, int height) {
        LayoutParams lp = getLayoutParams();
        lp.height = height;
        lp.width = width;
        setLayoutParams(lp);
    }

    public VideoView(Context context) {
        super(context);
        mContext = context;
        initVideoView();
    }

    public VideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
        mContext = context;
        initVideoView();
    }

    public VideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
        initVideoView();
    }

    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Log.i(TAG, "onMeasure: widthMeasureSpec = " + widthMeasureSpec + ", heightMeasureSpec = "
                + heightMeasureSpec);
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
        Log.i(TAG, "-----------in the onMeasure---------------width:" + width
                + " height:" + height + " mVideoWidth:" + mVideoWidth
                + " mVideoHeight:" + mVideoHeight + " widthMeasureSpec:"
                + widthMeasureSpec + " heightMeasureSpec:" + heightMeasureSpec);

        /*if (mVideoWidth > 0 && mVideoHeight > 0) {
            if (mVideoWidth * height > width * mVideoHeight) {
                Log.i(TAG, "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            } else if (mVideoWidth * height < width * mVideoHeight) {
                Log.i(TAG, "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            } else {
                Log.i(TAG, "aspect ratio is correct: " + width + "/" + height
                        + "=" + mVideoWidth + "/" + mVideoHeight);
            }
        }*/

        Log.i(TAG, "-----------in the onMeasure-------------->" + "width:"
                + width + " height:" + height);
        Log.i(TAG, "setting size: " + width + 'x' + height);
        setMeasuredDimension(width, height);
    }

    public int resolveAdjustedSize(int desiredSize , int measureSpec) {
        int result = desiredSize;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        switch (specMode) {
        case MeasureSpec.UNSPECIFIED:
            /*
             * Parent says we can be as big as we want. Just don't be larger
             * than max size imposed on ourselves.
             */
            result = desiredSize;
            break;

        case MeasureSpec.AT_MOST:
            /*
             * Parent says we can be as big as we want, up to specSize. Don't be
             * larger than specSize, and don't be larger than the max size
             * imposed on ourselves.
             */
            result = Math.min(desiredSize, specSize);
            break;

        case MeasureSpec.EXACTLY:
            // No choice. Do what we are told.
            result = specSize;
            break;
        }
        return result;
    }

    private boolean m_isInitlized = false;

    private void initVideoView() {
        mAudioFocusChangeListener = new AudioManager.OnAudioFocusChangeListener() {
            @Override
            public void onAudioFocusChange(int focusChange) {
                Log.d(TAG, "AudioFocusChangeListener: focusChange = " + focusChange);
                switch (focusChange) {
                case AudioManager.AUDIOFOCUS_GAIN:
                    mState.audioFocusGranted = true;
                    Log.d(TAG, "grant the audio Focus!");
                    break;
                case AudioManager.AUDIOFOCUS_LOSS:
                    mState.audioFocusGranted = false;
                    Log.d(TAG, "loss the audio Focus!");
                    //stopPlayback();
                    destroyPlayer();
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                    Log.d(TAG, "loss transient audio focus");
                    mState.audioFocusGranted = false;
                    mMediaPlayer.pause();
                    break;
                }
            }
        };
        if (m_isInitlized) {
            Log.d(TAG, "return and m_isInitlized = " + m_isInitlized);
            return;
        }
        Log.d(TAG,
                "initVideoView, run here. line="
                        + new Throwable().getStackTrace()[1].getLineNumber());
        Log.d(TAG, "initVideoView, this=" + this);
     // for test 20131204
//        display_man = new DisplayManager();
//        display_man = (DisplayManager)mContext.getSystemService(Context.DISPLAY_SERVICE);
        mVideoWidth = 0;
        mVideoHeight = 0;
        getHolder().addCallback(mSHCallback);
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_NORMAL);
        getHolder().setFormat(PixelFormat.RGBA_8888);
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        m_isInitlized = true;

        AudioManager am = (AudioManager) mContext
                .getSystemService(Context.AUDIO_SERVICE);
        int result = am.requestAudioFocus(mAudioFocusChangeListener, 
             // Use the music stream.
                AudioManager.STREAM_MUSIC,
                // Request permanent focus.
                AudioManager.AUDIOFOCUS_GAIN);
        Log.d(TAG, "initVideoView result = " + result);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            mState.audioFocusGranted = true;
            Log.d(TAG, "mState.audioFocusGranted = " + mState.audioFocusGranted);
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            mState.audioFocusGranted = false;
        }

    }

    public void setVideoPath(String path) {
        if (path == null) {
            return;
        }
        path = Converter.convertSpecialCharacters(path);
        Log.d(TAG, "setVideoPath: " + path);
        setVideoURI(Uri.parse(path));

    }

    // add liumeidong 20131115 begin
    public void setVideoPath(String path, Map<String, String> header) {
        if (path == null || header == null) {
            return;
        }
        path = Converter.convertSpecialCharacters(path);
        Log.d(TAG, "setVideoPath = " + path + ", header = " + header);
        setVideoURI(Uri.parse(path), header);
    }

    // add liumeidong 20131115 end

    public void setVideoURI(Uri uri) {
        mUri = uri;
        mStartWhenPrepared = false;
        mSeekWhenPrepared = 0;
        openVideo();
        requestLayout();
        invalidate();
    }

    // add liumeidong 20131115 begin
    public void setVideoURI(Uri uri, Map<String, String> header) {
        mUri = uri;
        mHeaders = header;
        mStartWhenPrepared = false;
        mSeekWhenPrepared = 0;
        openVideo();
        requestLayout();
        invalidate();
    }

    // add liumeidong 20131115 end

    public void stopPlayback() {
        Log.d(TAG, "stop Play back and stop video play + mMediaPlayer = " + mMediaPlayer);
        if (mMediaPlayer != null) {
            mMediaPlayer.stop();
            mMediaPlayer.release();
            // deleted by liumeidong 20131107
            StatusBarOperator.hideStatusBar(mContext); 
            mMediaPlayer = null;
        }
    }

    private void openVideo() {
        Log.d(TAG,
                "run here. line="
                        + new Throwable().getStackTrace()[1].getLineNumber());
        if (mUri == null || mSurfaceHolder == null) {
            // not ready for playback just yet, will try again later
            return;
        }
        Log.d(TAG,
                "run here. line="
                        + new Throwable().getStackTrace()[1].getLineNumber());
        // Tell the music playback service to pause
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");
        mContext.sendBroadcast(i);

        i = new Intent("com.android.music.videoOpened");
        i.putExtra("flag", "true");
        mContext.sendBroadcast(i);

        if (mMediaPlayer != null) {
            mMediaPlayer.reset();
            mMediaPlayer.release();
            // 20131107 liumeidong deteled
            StatusBarOperator.hideStatusBar(mContext); 
            mMediaPlayer = null;
        }
        try {
            Log.d(TAG,
                    "run here. line="
                            + new Throwable().getStackTrace()[1]
                                    .getLineNumber());
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnPreparedListener(mPreparedListener);
            mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
            mIsPrepared = false;
            mDuration = -1;
            mMediaPlayer.setOnCompletionListener(mCompletionListener);
//            mMediaPlayer
//                    .setOnFastBackwordCompleteListener(mFastBackwordCompleteListener);
            mMediaPlayer.setOnInfoListener(m3DModeReceivedListener);
            mMediaPlayer.setOnErrorListener(mErrorListener);
            mMediaPlayer.setOnBufferingUpdateListener(mBufferingUpdateListener);
            mCurrentBufferPercentage = 0;
            // mMediaPlayer.setDataSource(mContext, mUri);
            // add liumeidong 20131115 begin
            // mMediaPlayer.setDataSource(Uri.decode(mUri.toString()));
            Log.d(TAG, "mHeaders = " + mHeaders +  ", mUri.toString() = " + mUri.toString());
            if (mHeaders != null) {
                mMediaPlayer.setDataSource(Uri.decode(mUri.toString()), mHeaders);
            } else {
                mMediaPlayer.setDataSource(Uri.decode(mUri.toString()));
            }
            // add liumeidong 20131115 end
            Log.i(TAG, "surfaceholder=" + mSurfaceHolder);
            mSurfaceHolder.setFixedSize(getVideoWidth(), getVideoHeight());
            mMediaPlayer.setDisplay(mSurfaceHolder);
            // for test 20131204
//            mMediaPlayer.setSubDisplay(mSurfaceHolder);
            mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mMediaPlayer.setScreenOnWhilePlaying(true);
            mMediaPlayer.prepareAsync();
            // for test
            attachMediaController();
        } catch (IOException ex) {
            Log.w(TAG, "Unable to open content: " + mUri, ex);
            return;
        } catch (IllegalArgumentException ex) {
            Log.w(TAG, "Unable to open content: " + mUri, ex);
            return;
        }
    }

    // add temp for test

    public void setMediaController(MediaController controller) {
        if (mMediaController != null) {
            mMediaController.hide();
        }
        Log.d(TAG, "setMediaController: controller = " + controller);
        mMediaController = controller;
        attachMediaController();
    }

    private void attachMediaController() {
        Log.d(TAG, "attachMediaController: ");
        if (mMediaPlayer != null && mMediaController != null) {
            mMediaController.setMediaPlayer(this);
            View anchorView = this.getParent() instanceof View ? (View) this
                    .getParent() : this;
            mMediaController.setAnchorView(anchorView);
            mMediaController.setEnabled(mIsPrepared);
        }
    }

    MediaPlayer.OnVideoSizeChangedListener mSizeChangedListener = new MediaPlayer.OnVideoSizeChangedListener() {
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
             mVideoWidth = mp.getVideoWidth();
             mVideoHeight = mp.getVideoHeight();
            if (mVideoWidth != 0 && mVideoHeight != 0) {
                 getHolder().setFixedSize(mVideoWidth, mVideoHeight);
            }
        }
    };

    MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mp) {
            // briefly show the mediacontroller
            mIsPrepared = true;
            if (mOnPreparedListener != null) {
                mOnPreparedListener.onPrepared(mMediaPlayer);
                return;
            }
            mVideoWidth = mp.getVideoWidth();
            mVideoHeight = mp.getVideoHeight();
            Log.i(TAG, "------------------------------>mVideoWidth:"
                    + mVideoWidth + " mVideoHeight:" + mVideoHeight
                    + " mSurfaceWidth:" + mSurfaceWidth + " mSurfaceHeight:"
                    + mSurfaceHeight);

            if (mVideoWidth != 0 && mVideoHeight != 0) {
                // Log.i("@@@@", "video size: " + mVideoWidth +"/"+
                // mVideoHeight);
                getHolder().setFixedSize(mVideoWidth, mVideoHeight);
                // if (mSurfaceWidth == mVideoWidth && mSurfaceHeight ==
                // mVideoHeight) {
                // We didn't actually change the size (it was already at the
                // size
                // we need), so we won't get a "surface changed" callback, so
                // start the video here instead of in the callback.
                Log.i(TAG, "------------------------->mStartWhenPrepared:"
                        + mStartWhenPrepared + " mSeekWhenPrepared:"
                        + mSeekWhenPrepared);
                if (mSeekWhenPrepared != 0) {
                    mMediaPlayer.seekTo(mSeekWhenPrepared);
                    mSeekWhenPrepared = 0;
                }

                if (mStartWhenPrepared) {
                    Log.d(TAG, "mStartWhenPrepared = " + mStartWhenPrepared + ", play the video");
                    mMediaPlayer.start();
                    mStartWhenPrepared = false;
                }
                // }
            } else {
                // We don't know the video size yet, but should start anyway.
                // The video size might be reported to us later.
                if (mSeekWhenPrepared != 0) {
                    mMediaPlayer.seekTo(mSeekWhenPrepared);
                    mSeekWhenPrepared = 0;
                }
                if (mStartWhenPrepared) {
                    mMediaPlayer.start();
                    mStartWhenPrepared = false;
                }
            }
        }
    };

    private MediaPlayer.OnCompletionListener mCompletionListener = new MediaPlayer.OnCompletionListener() {
        public void onCompletion(MediaPlayer mp) {
            if (mOnCompletionListener != null) {
                mOnCompletionListener.onCompletion(mMediaPlayer);
            }
        }
    };

//    private MediaPlayer.OnFastBackwordCompleteListener mFastBackwordCompleteListener = new MediaPlayer.OnFastBackwordCompleteListener() {
//        public void onFastBackwordComplete(MediaPlayer mp) {
//            if (mOnFastBackwordCompleteListener != null) {
//                mOnFastBackwordCompleteListener
//                        .onFastBackwordComplete(mMediaPlayer);
//            }
//        }
//    };

    private MediaPlayer.OnInfoListener m3DModeReceivedListener = new MediaPlayer.OnInfoListener() {
        public boolean onInfo(MediaPlayer mp, int what, int extra) {
            if (mOn3DModeReceivedListener != null) {
                return mOn3DModeReceivedListener.onInfo(mMediaPlayer, what,
                        extra);
            }
            return false;
        }
    };

    private MediaPlayer.OnErrorListener mErrorListener = new MediaPlayer.OnErrorListener() {
        public boolean onError(MediaPlayer mp, int framework_err, int impl_err) {
            Log.d(TAG, "Error: " + framework_err + "," + impl_err);

            /* If an error handler has been supplied, use it and finish. */
            if (mOnErrorListener != null) {
                if (mOnErrorListener.onError(mMediaPlayer, framework_err,
                        impl_err)) {
                    return true;
                }
            }

            /*
             * Otherwise, pop up an error dialog so the user knows that
             * something bad has happened. Only try and pop up the dialog if
             * we're attached to a window. When we're going away and no longer
             * have a window, don't bother showing the user an error.
             */
            if (getWindowToken() != null) {
                Resources r = mContext.getResources();
                int messageId;

                if (framework_err == MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
                    messageId = com.android.internal.R.string.VideoView_error_text_invalid_progressive_playback;
                } else {
                    messageId = com.android.internal.R.string.VideoView_error_text_unknown;
                }

                new AlertDialog.Builder(mContext)
                        .setTitle(
                                com.android.internal.R.string.VideoView_error_title)
                        .setMessage(messageId)
                        .setPositiveButton(
                                com.android.internal.R.string.VideoView_error_button,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                            int whichButton) {
                                        /**
                                         * If we get here, there is no onError
                                         * listener, so at least inform them
                                         * that the video is over.
                                         */
                                        if (mOnCompletionListener != null) {
                                            mOnCompletionListener
                                                    .onCompletion(mMediaPlayer);
                                        }
                                    }
                                }).setCancelable(false).show();

            }
            return true;
        }
    };

    public void setSubDisplay(SurfaceHolder holder) {
        if (mMediaPlayer != null) {
//            mMediaPlayer.setSubDisplay(holder);
//            mMediaPlayer.setDisplay(holder);
        }
    }

    private MediaPlayer.OnBufferingUpdateListener mBufferingUpdateListener = new MediaPlayer.OnBufferingUpdateListener() {
        public void onBufferingUpdate(MediaPlayer mp, int percent) {
            Log.d(TAG, "OnBufferingUpdateListener: percent " + percent);
            mCurrentBufferPercentage = percent;
        }
    };

    // add 20131106 begin
    public void setOnInfoListener(MediaPlayer.OnInfoListener listener) {
        mOnInfoListener = listener;
    }

    // add 20131106 end

    /**
     * Register a callback to be invoked when the media file is loaded and ready
     * to go.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnPreparedListener(MediaPlayer.OnPreparedListener l) {
        mOnPreparedListener = l;
    }

    /**
     * Register a callback to be invoked when the end of a media file has been
     * reached during playback.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnCompletionListener(MediaPlayer.OnCompletionListener l) {
        mOnCompletionListener = l;
    }

    // add liumeidong
    public void setOnBufferingUpdateListener(
            MediaPlayer.OnBufferingUpdateListener l) {
        onBufferingUpdateListener = l;
    }

    public int get3DMode() {
        return 0;
        // return mMediaPlayer.get3DMode();
    }

    public int set3DMode(int mode) {
        return mode;
//         return mMediaPlayer.set3DMode(mode);
    }

    public int getStereoMode() {
        return 0;
//      for test 20131204
//        return display_man.getStereoMode();
    }

    public int setStereoMode(int mode) {

        return mode;
//         for test 20131204
//        return display_man.setStereoMode(mode);
    }

//    public void setOnFastBackwordCompleteListener(
//            MediaPlayer.OnFastBackwordCompleteListener l) {
//        mOnFastBackwordCompleteListener = l;
//    }

    public void setOn3DModeReceivedListener(MediaPlayer.OnInfoListener l) {
        mOn3DModeReceivedListener = l;
    }

    /**
     * Register a callback to be invoked when an error occurs during playback or
     * setup. If no listener is specified, or if the listener returned false,
     * VideoView will inform the user of any errors.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnErrorListener(MediaPlayer.OnErrorListener l) {
        mOnErrorListener = l;
    }

    SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {
        public void surfaceChanged(SurfaceHolder holder, int format, int w,
                int h) {

            mSurfaceWidth = w;
            mSurfaceHeight = h;
            Log.i(TAG,
                    "---------------in the surfaceChanged------------------mSurfaceWidth:"
                            + mSurfaceWidth + " mSurfaceHeight:"
                            + mSurfaceHeight + " mIsPrepared:" + mIsPrepared
                            + " mVideoWidth:" + mVideoWidth + " mVideoHeight:"
                            + mVideoHeight);
            if (mMediaPlayer != null && mIsPrepared && mVideoWidth == w
                    && mVideoHeight == h) {
                if (mSeekWhenPrepared != 0) {
                    mMediaPlayer.seekTo(mSeekWhenPrepared);
                    mSeekWhenPrepared = 0;
                }
                // begin delete by xiongCuiFan/zhouyong (sometimes play in back
                // when pop
                // continue play dialog ) 2011.10.24 hoperun
                // mMediaPlayer.start();
                // end delete by xiongCuiFan/zhouyong
            }
        }

        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG,
                    "surfaceCreated run here. line="
                            + new Throwable().getStackTrace()[1]
                                    .getLineNumber());
            mSurfaceHolder = holder;
            openVideo();
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG,
                    "surfaceDestroyed run here. line="
                            + new Throwable().getStackTrace()[1]
                                    .getLineNumber());
            destroyPlayer();
        }
    };

    /**
     * 
     * @param request
     * @param reply
     * @return
     * @author xiongCuiFan
     */
    // modifty 20131204
    public void invoke(Parcel request, Parcel reply) {
      try {
        if (mMediaPlayer != null && mIsPrepared) {
            mMediaPlayer.invoke(request, reply);
        }
      } catch (RuntimeException e) {
          Log.d(TAG, "invoke: error");
      }
//        return null;
    }

    /**
     * 
     * @author xiongCuiFan
     */
    public void resume() {
        if (mMediaPlayer != null && mIsPrepared) {
            // mMediaPlayer.resume();
            play();
            CommonData.isResume = true; // add by xiongCuiFan/zhouyong
                                        // (sometimes
                                        // maybe can not pause) 2011.10.27
                                        // hoperun
        }
    }

    // instead of mMediaPlayer.resume()
    public void play() {
        Parcel requestParcel = Parcel.obtain();
        requestParcel.writeInterfaceToken("android.media.IMediaPlayer");
        requestParcel.writeInt(CMD_STOP_FASTPLAY);
        Parcel replayParcel = Parcel.obtain();
        invoke(requestParcel, replayParcel);
    }

    public void start() {
        if (mMediaPlayer != null && mIsPrepared && mState.audioFocusGranted) {
            Log.d(TAG, "start the video");
            mMediaPlayer.start();
            mStartWhenPrepared = false;
        } else {
            mStartWhenPrepared = true;
        }
    }

    public void pause() {
        if (mMediaPlayer != null && mIsPrepared) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.pause();
            }
        }
        mStartWhenPrepared = false;
    }

    public int getDuration() {
        if (mMediaPlayer != null && mIsPrepared) {
            if (mDuration > 0) {
                return mDuration;
            }
            mDuration = mMediaPlayer.getDuration();
            return mDuration;
        }
        mDuration = -1;
        return mDuration;
    }

    public int getCurrentPosition() {
        if (mMediaPlayer != null && mIsPrepared) {
            return mMediaPlayer.getCurrentPosition();
        }
        return 0;
    }

    public void seekTo(int msec) {
        if (mMediaPlayer != null && mIsPrepared) {
            mMediaPlayer.seekTo(msec);
        } else {
            mSeekWhenPrepared = msec;
        }
    }

    public boolean isPlaying() {
        if (mMediaPlayer != null && mIsPrepared) {
            Log.d(TAG, "isPlaying(): mMediaPlayer.isPlaying() = " + mMediaPlayer.isPlaying());
            return mMediaPlayer.isPlaying();
        }
        return false;
    }

    public int getBufferPercentage() {
        if (mMediaPlayer != null) {
            return mCurrentBufferPercentage;
        }
        return 0;
    }

    public boolean canPause() {
        return false;
    }

    public boolean canSeekBackward() {
        return false;
    }

    public boolean canSeekForward() {
        return false;
    }

    public void destroyPlayer() {
        Log.i(TAG,
                "-----------in the destroyPlayer-------------mSurfaceHolder:"
                        + mSurfaceHolder + " mMediaPlayer:" + mMediaPlayer);
        if (mSurfaceHolder != null) {
            mSurfaceHolder = null;
            Intent i = new Intent("com.android.music.videoOpened");
            i.putExtra("flag", "false");
            mContext.sendBroadcast(i);
        }
        if (mMediaPlayer != null) {
            mMediaPlayer.reset();
            mMediaPlayer.release();
            // 20131107 liumeidong deteled
            StatusBarOperator.hideStatusBar(mContext);
            mMediaPlayer = null;
        }
    }

    public void setLRVolume(float arg0, float arg1) {
        if (mMediaPlayer != null && mIsPrepared) {
            Log.d(TAG, "setLRVolume arg0 = " + arg0 + ", arg1 = " + arg1);
            mMediaPlayer.setVolume(arg0, arg1);
        }
    }

    private AudioManager.OnAudioFocusChangeListener mAudioFocusChangeListener;
    enum PlayState {
        IDLE, PLAY, PAUSE, STOP
      }
    private static final int UNKNOWN = -1;
    PlayState state = PlayState.IDLE;
    public boolean looping;

    class MusicPlayerStates {
        boolean audioFocusGranted = false;
        boolean userInitiatedState = false;
        boolean wasPlayingWhenTransientLoss = false;
        boolean released = false;
        int lastKnownAudioFocusState = UNKNOWN;
    }
    private MusicPlayerStates mState = new MusicPlayerStates();

}
