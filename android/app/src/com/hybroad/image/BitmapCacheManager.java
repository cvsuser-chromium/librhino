package com.hybroad.image;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.SoftReference;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Hashtable;

import org.apache.http.conn.ConnectTimeoutException;

import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import com.hybroad.util.Converter;

public class BitmapCacheManager {
    private final static String TAG = "----BitmapCacheManager----";
    private volatile static BitmapCacheManager cache;
    private Hashtable<String, MySoftRef> referencesHashTable;
    private ReferenceQueue<BitmapPackage> garbageQueue;
    private int mTargetWidth = 1280;
    private int mTargetHeight = 720;

    private class MySoftRef extends SoftReference<BitmapPackage> {
        private String _key;

        public MySoftRef(BitmapPackage bitmapPackage,
                ReferenceQueue<BitmapPackage> q, String key) {
            super(bitmapPackage, q);
            Log.d(TAG, "MySoftRef construction method key = " + key);
            _key = key;
        }
    }

    private BitmapCacheManager(int targetPixelsWidth, int targetPixelsHeight) {
        referencesHashTable = new Hashtable<String, MySoftRef>();
        garbageQueue = new ReferenceQueue<BitmapPackage>();
        mTargetWidth = targetPixelsWidth;
        mTargetHeight = targetPixelsHeight;
        Log.d(TAG, "BitmapCacheManager construction method targetPixelsWidth = " + targetPixelsWidth
                + ", targetPixelsHeight = " + targetPixelsHeight);
    }

    public static BitmapCacheManager getInstance(int targetPixelsWidth,
            int targetPixelsHeight) {
        Log.d(TAG, "BitmapCacheManager getInstance targetPixelsWidth = " + targetPixelsWidth
                + ", targetPixelsHeight = " + targetPixelsHeight);
        if (cache == null) {
            synchronized (BitmapCacheManager.class) {
                if (cache == null) {
                    cache = new BitmapCacheManager(targetPixelsWidth,
                            targetPixelsHeight);
                }
            }
        }
        return cache;
    }

    private void addCacheBitmap(BitmapPackage bitmapPackage, String key) {
        cleanGarbageCache();
        Log.d(TAG, "addCacheBitmap: key = " + key + ", bitmapPackage = " + bitmapPackage);
        MySoftRef ref = new MySoftRef(bitmapPackage, garbageQueue, key);
        referencesHashTable.put(key, ref);
    }

    public void deleteCacheByKey(String key) {
        // remove the key/value from the referencesHashTable
        Log.d(TAG, "deleteCacheByKey: key = " + key);
        referencesHashTable.remove(key);
    }

    private void cleanGarbageCache() {
        Log.d(TAG, "-----cleanGarbageCache()-----");
        MySoftRef ref = null;
        while ((ref = (MySoftRef) garbageQueue.poll()) != null) {
            referencesHashTable.remove(ref._key);
        }
        // add temp for test begin
        // File file = new File("var/");
        // release(file);
        // add temp for test end
    }

    // add temp for test begin
    // deleted the file in var resource
    public void release(File file) {
        if (!file.exists()) {
            return;
        }
        Log.d(TAG, "release the resources");
        File[] files = file.listFiles();
        if (files.length > 0) {
            for (File f : files) {
                Log.d(TAG, "release f = " + f);
                f.delete();
            }
        }
    }

    // add temp for test end

    public void clearCache() {
        Log.d(TAG, "clearCache()----");
        cleanGarbageCache();
        referencesHashTable.clear();
        System.gc();
        System.runFinalization();
    }

    @SuppressWarnings("resource")
    public BitmapPackage getBitmapPackageFromPath(Context context,
            String filePath) {
        Log.d(TAG, "getBitmapPackageFromPath(): filePath = " + filePath);
        if (TextUtils.isEmpty(filePath) || context == null) {
            return null;
        }
        String transformedPath = Converter.convertSpecialCharacters(filePath);

        if (referencesHashTable.containsKey(filePath)) {
            Log.d(TAG, "referencesHashTable containsKey with the filePath = " + filePath);
            MySoftRef ref = (MySoftRef) referencesHashTable.get(filePath);
            BitmapPackage bitmapPackage = ref.get();
            if (bitmapPackage != null) {
                if (!bitmapPackage.bitmapIsNull()) {
                    return bitmapPackage;
                } else {
                    deleteCacheByKey(filePath);
                }
            } else {
                deleteCacheByKey(filePath);
            }
        }

        BitmapPackage mBitmapPackage = new BitmapPackage();
        ContentResolver cr = context.getContentResolver();
        InputStream inputStream = null;
        Bitmap bitmap = null;
        BitmapFactory.Options options = new BitmapFactory.Options();
        HttpURLConnection httpURLConnection = null;
        URL url = null;

        try {
            // First, read the actual size of the image
            options.inSampleSize = 1;
            options.inJustDecodeBounds = true;

            if (transformedPath.startsWith("http://")) {
                url = new URL(transformedPath);
                httpURLConnection = (HttpURLConnection) url.openConnection();
                if (httpURLConnection != null) {
                    httpURLConnection.setConnectTimeout(5000);
                    httpURLConnection.setReadTimeout(5000);
                    httpURLConnection.setRequestMethod("GET");
                    int responseCode;
                    if (200 == (responseCode = httpURLConnection
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

            BitmapFactory.decodeStream(inputStream, null, options);
            Log.d(TAG, "getBitmapPackageFromPath() options.outWidth = "
                    + options.outWidth + ", options.outHeight = "
                    + options.outHeight);
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

            // Then read the image content
            options.inJustDecodeBounds = false;
            // options.inSampleSize = ImageUtils.computeSampleSize(options,
            // minSideLength, maxNumOfPixels);
            if (options.outWidth > mTargetWidth
                    || options.outHeight > mTargetHeight) {
                int heightSampleRate = (int) Math
                        .ceil(((double) options.outHeight / (double) mTargetHeight));
                int widthSampleRate = (int) Math.ceil((double) options.outWidth
                        / (double) mTargetWidth);
                options.inSampleSize = heightSampleRate > widthSampleRate ? heightSampleRate
                        : widthSampleRate;
            }

            mBitmapPackage.setOriginalWidth(options.outWidth);
            mBitmapPackage.setOriginalHeight(options.outHeight);
            mBitmapPackage.setSampleValue(options.inSampleSize);
            Log.d(TAG, "getBitmapPackageFromPath options.inSampleSize: "
                    + options.inSampleSize);

            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            if (transformedPath.startsWith("http://")) {
                httpURLConnection = (HttpURLConnection) url.openConnection();
                if (httpURLConnection != null) {
                    httpURLConnection.setConnectTimeout(5000);
                    httpURLConnection.setReadTimeout(5000);
                    httpURLConnection.setRequestMethod("GET");
                    int responseCode;
                    if (200 == (responseCode = httpURLConnection
                            .getResponseCode())) {
                        inputStream = httpURLConnection.getInputStream();
                    } else {
                        Log.e(TAG,
                                "getBitmapPackageFromPath connect failure! responseCode: "
                                        + responseCode);
                        if (httpURLConnection != null) {
                            httpURLConnection.disconnect();
                            httpURLConnection = null;
                        }
                        return null;
                    }
                }
            } else if (transformedPath.startsWith("/")) {
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

            Log.d(TAG, "getBitmapPackageFromPath() filePath: " + filePath);
            bitmap = BitmapFactory.decodeStream(inputStream, null, options);
            Log.d(TAG, "getBitmapPackageFromPath() bitmap: " + bitmap);
            if (inputStream != null) {
                inputStream.close();
            }
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
                httpURLConnection = null;
            }
        } catch (ConnectTimeoutException e) {
            Log.e(TAG, "ConnectTimeoutException", e);
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

        if (bitmap != null) {
            mBitmapPackage.setBitmap(bitmap);
            this.addCacheBitmap(mBitmapPackage, filePath);
        }
        return mBitmapPackage;
    }

}
