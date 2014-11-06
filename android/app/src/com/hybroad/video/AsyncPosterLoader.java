package com.hybroad.video;

import java.lang.ref.SoftReference;
import java.util.HashMap;

import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.ThumbnailUtils;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore.Video.Thumbnails;

public class AsyncPosterLoader {
  private HashMap<String, SoftReference<Drawable>> imageCacheHashMap;

  public AsyncPosterLoader() {
    imageCacheHashMap = new HashMap<String, SoftReference<Drawable>>();
  }

  private class PosterImageLoaderMSG {
    private Drawable drawable = null;
    private String imageTag = null;
    private PosterCallback posterCallback = null;
  }

  final Handler callbackHandler = new Handler() {
    @Override
    public void handleMessage(Message message) {

      PosterCallback posterCallback = ((PosterImageLoaderMSG) message.obj).posterCallback;
      String imageTag = ((PosterImageLoaderMSG) message.obj).imageTag;
      Drawable drawable = ((PosterImageLoaderMSG) message.obj).drawable;
      posterCallback.imageLoaded(drawable, imageTag);

      imageCacheHashMap.put(imageTag, new SoftReference<Drawable>(drawable));
    }
  };

  public synchronized Drawable loadDrawableFromCache(final String imageTag) {
    if (imageCacheHashMap != null && imageCacheHashMap.containsKey(imageTag)) {
      SoftReference<Drawable> softReference = imageCacheHashMap.get(imageTag);
      Drawable drawable = softReference.get();
      if (drawable != null) {
        return drawable;
      }
    }
    return null;
  }

  @SuppressWarnings("deprecation")
  public void loadDrawableFromVideo(final int position, final String imageTag,
      final PosterCallback posterCallback) {
    PosterImageLoaderMSG msg = new PosterImageLoaderMSG();
    msg.imageTag = imageTag;
    Bitmap bitmap = ThumbnailUtils.createVideoThumbnail(imageTag,
        Thumbnails.MICRO_KIND);
    if (bitmap != null) {
      msg.drawable = new BitmapDrawable(bitmap);
    }
    msg.posterCallback = posterCallback;
    if (msg.drawable != null) {
      Message message = callbackHandler.obtainMessage(0, msg);
      callbackHandler.sendMessage(message);
    }
  }
}
