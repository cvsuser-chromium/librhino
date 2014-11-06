package com.hybroad.music;

import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import com.hybroad.media.R;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

public class MusicUtils {
    private final static String TAG = "--MusicUtils--";
    private static int mTargetWidth = 260;
    private static int mTargetHeight = 260;

    public static Bitmap getArtworkFromFile(String filePath) {
        if (filePath == null) {
            return null;
        }
        Bitmap bitmap = null;
        byte[] tempByteArray;

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(filePath);
            tempByteArray = retriever.getEmbeddedPicture();
            if (tempByteArray != null) {
                BitmapFactory.Options options = new BitmapFactory.Options();
                options.inSampleSize = 1;
                options.inJustDecodeBounds = true;
                BitmapFactory.decodeByteArray(tempByteArray, 0,
                        tempByteArray.length, options);
                if (options.mCancel || options.outWidth == -1
                        || options.outHeight == -1) {
                    Log.d(TAG,
                            "options.mCancel || options.outWidth == -1 || options.outHeight == -1");
                    return null;
                }
                Log.d(TAG, "options.outWidth = " + options.outWidth
                        + ", options.outHeight = " + options.outHeight);
                options.inJustDecodeBounds = false;
                if (options.outWidth > mTargetWidth
                        || options.outHeight > mTargetHeight) {
                    int heightSampleRate = (int) Math
                            .ceil(((double) options.outHeight / (double) mTargetHeight));
                    int widthSampleRate = (int) Math
                            .ceil((double) options.outWidth
                                    / (double) mTargetWidth);
                    options.inSampleSize = heightSampleRate > widthSampleRate ? heightSampleRate
                            : widthSampleRate;
                }
                Log.d(TAG, "options.inSampleSize: " + options.inSampleSize);
                bitmap = BitmapFactory.decodeByteArray(tempByteArray, 0,
                        tempByteArray.length);
            }
        } catch (Exception e) {
            // TODO: handle exception
            e.printStackTrace();
        }

        return bitmap;
    }

    public static Bitmap getArtwork(Context context, int song_id, int album_id) {
        if (context == null || album_id < 0) {
            return null;
        }

        ContentResolver res = context.getContentResolver();
        Uri uri = ContentUris.withAppendedId(
                Uri.parse("content://media/external/audio/albumart"), album_id);
        if (uri != null) {
            InputStream in = null;
            BitmapFactory.Options sBitmapOptions = new BitmapFactory.Options();
            sBitmapOptions.inPreferredConfig = Bitmap.Config.RGB_565;
            sBitmapOptions.inDither = false;
            try {
                in = res.openInputStream(uri);
                return BitmapFactory.decodeStream(in, null, sBitmapOptions);
            } catch (FileNotFoundException ex) {
                // TODO: handle exception
                Bitmap bm = getArtworkFromFile(context, song_id, album_id);
                if (bm != null) {
                    if (bm.getConfig() == null) {
                        bm = bm.copy(Bitmap.Config.RGB_565, false);
                        if (bm == null) {
                            return getDefaultArtwork(context);
                        }
                    }
                }
                return bm;
            } finally {
                try {
                    if (in != null) {
                        in.close();
                    }
                } catch (IOException ex) {
                }
            }
        }

        return null;
    }

    private static Bitmap getArtworkFromFile(Context context, int song_id,
            int album_id) {
        Bitmap bm = null;

        if (album_id < 0 && song_id < 0) {
            throw new IllegalArgumentException(
                    "Must specify an album or a song id");
        }

        try {
            if (album_id < 0) {
                Uri uri = Uri.parse("content://media/external/audio/media/"
                        + song_id + "/albumart");
                ParcelFileDescriptor pfd = context.getContentResolver()
                        .openFileDescriptor(uri, "r");
                if (pfd != null) {
                    FileDescriptor fd = pfd.getFileDescriptor();
                    bm = BitmapFactory.decodeFileDescriptor(fd);
                }
            } else {
                Uri uri = ContentUris.withAppendedId(
                        Uri.parse("content://media/external/audio/albumart"),
                        album_id);
                ParcelFileDescriptor pfd = context.getContentResolver()
                        .openFileDescriptor(uri, "r");
                if (pfd != null) {
                    FileDescriptor fd = pfd.getFileDescriptor();
                    bm = BitmapFactory.decodeFileDescriptor(fd);
                }
            }
        } catch (IllegalStateException ex) {
        } catch (FileNotFoundException ex) {
        }

        return bm;
    }

    private static Bitmap getDefaultArtwork(Context context) {
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeStream(context.getResources()
                .openRawResource(R.drawable.music_thumbnail_default), null,
                opts);
    }
}
