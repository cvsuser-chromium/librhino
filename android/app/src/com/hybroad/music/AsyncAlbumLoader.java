package com.hybroad.music;

import java.lang.ref.SoftReference;
import java.util.HashMap;

import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;

public class AsyncAlbumLoader {
    private HashMap<String, SoftReference<Drawable>> imageCacheHashMap;
    private Bitmap mBitmap = null;
    private BitmapDrawable mBmpDraw = null;

    public AsyncAlbumLoader() {
        imageCacheHashMap = new HashMap<String, SoftReference<Drawable>>();
    }

    private class albumImageLoaderMSG {
        private Drawable albumDrawable = null;
        private String imageTag = null;
        private AlbumCallback albumImageCallback = null;
    }

    final Handler callbackHandler = new Handler() {
        public void handleMessage(Message message) {

            AlbumCallback albumImageCallback = ((albumImageLoaderMSG) message.obj).albumImageCallback;
            String imageTag = ((albumImageLoaderMSG) message.obj).imageTag;
            Drawable albumDrawable = ((albumImageLoaderMSG) message.obj).albumDrawable;
            albumImageCallback.imageLoaded(albumDrawable, imageTag);

            imageCacheHashMap.put(imageTag, new SoftReference<Drawable>(
                    albumDrawable));
        }
    };

    public synchronized Drawable loadDrawableFromCache(final String imageTag) {
        if (imageCacheHashMap != null
                && imageCacheHashMap.containsKey(imageTag)) {
            SoftReference<Drawable> softReference = imageCacheHashMap
                    .get(imageTag);
            Drawable drawable = softReference.get();
            if (drawable != null) {
                return drawable;
            }
        }
        return null;
    }

    @SuppressWarnings("deprecation")
    public void loadDrawableFromSong(final int position, final String imageTag,
            final AlbumCallback imageCallback) {
        albumImageLoaderMSG msg = new albumImageLoaderMSG();
        msg.imageTag = imageTag;
        // mBitmap = MusicUtils.getArtwork(mContext,
        // audiosList.get(position).song_id, audiosList.get(position).album_id);
        mBitmap = MusicUtils.getArtworkFromFile(imageTag);
        if (mBitmap != null) {
            mBmpDraw = new BitmapDrawable(mBitmap);
            if (mBmpDraw != null) {
                msg.albumDrawable = mBmpDraw;
            }
        } else {
            MusicPlayList.sCurrentMusicList.get(position).embededImageExists = 0;
        }
        msg.albumImageCallback = imageCallback;
        if (msg.albumDrawable != null) {
            Message message = callbackHandler.obtainMessage(0, msg);
            callbackHandler.sendMessage(message);
        }
    }
}
