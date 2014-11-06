package com.hybroad.image;

import java.lang.ref.SoftReference;
import java.util.HashMap;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class AsyncImageLoader {
    private Context mContext;
    private HashMap<String, SoftReference<Drawable>> imageCacheHashMap;
    private Bitmap mBitmap = null;
    private static final String TAG = "AsyncImageLoader";
    private BitmapDrawable mBmpDraw = null;
    private final int targetThumbnailWidth = 128;
    private final int targetThumbnailHeight = 128;

    public AsyncImageLoader(Context context) {
        Log.d(TAG, "AsyncImageLoader: context = " + context);
        imageCacheHashMap = new HashMap<String, SoftReference<Drawable>>();
        mContext = context;
    }

    private class photoImageLoaderMSG {
        private Drawable photoDrawable = null;
        private String imageTag = null;
        private ImageCallback photoImageCallback = null;
    }

    @SuppressLint("HandlerLeak")
    final Handler callbackHandler = new Handler() {
        public void handleMessage(Message message) {

            ImageCallback photoImageCallback = ((photoImageLoaderMSG)message.obj).photoImageCallback;
            String imageTag = ((photoImageLoaderMSG)message.obj).imageTag;
            Drawable photoDrawable = ((photoImageLoaderMSG)message.obj).photoDrawable;
            photoImageCallback.imageLoaded(photoDrawable, imageTag);

            imageCacheHashMap.put(imageTag, new SoftReference<Drawable>(photoDrawable));
        }
    };

    public  synchronized Drawable loadDrawableFromCache(final String imageTag) {
        if (imageCacheHashMap != null && imageCacheHashMap.containsKey(imageTag)) {
            SoftReference<Drawable> softReference = imageCacheHashMap.get(imageTag);
            Log.d(TAG, "loadDrawableFromCache: imageTag = " + imageTag);
            Drawable drawable = softReference.get();
            if (drawable != null) {
                return drawable;
            }
        }
        return null;
    }

    @SuppressWarnings("deprecation")
	public void loadDrawableFromPhoto(final int position, final String imageTag, final ImageCallback imageCallback) {
        photoImageLoaderMSG msg = new photoImageLoaderMSG();
        msg.imageTag = imageTag;
        mBitmap = ImageUtils.getImageThumbnail(mContext, imageTag, targetThumbnailWidth, targetThumbnailHeight);
        if (mBitmap != null) {
            mBmpDraw = new BitmapDrawable(mBitmap);
            if (mBmpDraw != null) {
                msg.photoDrawable = mBmpDraw;
            }
        }
        Log.d(TAG, "loadDrawableFromPhoto: position = " + position + ", imageTag = "
                + imageTag + ", imageCallback = " + imageCallback);
        msg.photoImageCallback = imageCallback;
        if (msg.photoDrawable != null) {
            Message message = callbackHandler.obtainMessage(0, msg);
            callbackHandler.sendMessage(message);
        }
    }

}
