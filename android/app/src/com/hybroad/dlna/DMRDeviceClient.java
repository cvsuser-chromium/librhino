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

package com.hybroad.dlna;

import java.lang.ref.WeakReference;

import org.chromium.base.CalledByNative;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.PowerManager;
import android.util.Log;

public class DMRDeviceClient {
    static {
        // System.loadLibrary("dlna_jni");
        native_init();
    }

    private final static String TAG = "DMRDeviceClient";
    // Name of the remote interface for the media player. Must be kept
    // in sync with the 2nd parameter of the IMPLEMENT_META_INTERFACE
    // macro invocation in IMediaPlayer.cpp
    private final static String IDMR_DEVICE = "com.hybroad.IDMRDeviceClient";

    // accessed by native methods
    // DMPDevice class lives in C++ side.
    private int mNativeDMRDeviceClient;
    // JNIDMPDeviceListener object lived in c++ side.
    private int mListenerContext; // accessed by native methods

    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;

    static Context mContext = null;

    /**
     * Default constructor. Consider using one of the create() methods for
     * synchronously instantiating a MediaPlayer from a Uri or resource.
     * <p>
     * When done with the DMRDeviceClient, you should call {@link #release()},
     * to free the resources. If not released, too many MediaPlayer instances
     * may result in an exception.
     * </p>
     */
    private DMRDeviceClient() {

        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        /*
         * Native setup requires a weak reference to our object. It's easier to
         * create it here than in C++.
         */
        native_setup(new WeakReference<DMRDeviceClient>(this), "/data/data/com.hybroad.media/files/dlna_root", "dinglei_dmr");
        Log.d(TAG, "create DMRDeviceClient!");
    }

    /**
     * Create a request parcel which can be routed to the native media player
     * using {@link #invoke(Parcel, Parcel)}. The Parcel returned has the proper
     * InterfaceToken set. The caller should not overwrite that token, i.e it
     * can only append data to the Parcel.
     * 
     * @return A parcel suitable to hold a request for the native player.
     *         {@hide
   * }
     */
    public Parcel newRequest() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInterfaceToken(IDMR_DEVICE);
        return parcel;
    }

    /**
     * Invoke a generic method on the native player using opaque parcels for the
     * request and reply. Both payloads' format is a convention between the java
     * caller and the native player. Must be called after setDataSource to make
     * sure a native player exists.
     * 
     * @param request
     *            Parcel with the data for the extension. The caller must use
     *            {@link #newRequest()} to get one.
     * 
     * @param reply
     *            Output parcel with the data returned by the native player.
     * 
     * @return The status code see utils/Errors.h {@hide}
     */
    public int invoke(Parcel request, Parcel reply) {
        int retcode = native_invoke(request, reply);
        reply.setDataPosition(0);
        return retcode;
    }

    /**
     * Convenience method to create a MediaPlayer for a given resource id. On
     * success, {@link #prepare()} will already have been called and must not be
     * called again.
     * <p>
     * When done with the MediaPlayer, you should call {@link #release()}, to
     * free the resources. If not released, too many MediaPlayer instances will
     * result in an exception.
     * </p>
     * 
     * @param context
     *            the Context to use
     * @param resid
     *            the raw resource id (<var>R.raw.&lt;something></var>) for the
     *            resource to use as the datasource
     * @return a MediaPlayer object, or null if creation failed
     */
    public static DMRDeviceClient create(Context context) {
        mContext = context;
        try {

            DMRDeviceClient dmp = new DMRDeviceClient();
            return dmp;
        } catch (IllegalArgumentException ex) {
            Log.d(TAG, "create failed:", ex);
            // fall through
        } catch (SecurityException ex) {
            Log.d(TAG, "create failed:", ex);
            // fall through
        }
        return null;
    }

    /**
     * Releases resources associated with this MediaPlayer object. It is
     * considered good practice to call this method when you're done using the
     * MediaPlayer. In particular, whenever an Activity of an application is
     * paused (its onPause() method is called), or stopped (its onStop() method
     * is called), this method should be invoked to release the MediaPlayer
     * object, unless the application has a special need to keep the object
     * around. In addition to unnecessary resources (such as memory and
     * instances of codecs) being held, failure to call this method immediately
     * if a MediaPlayer object is no longer needed may also lead to continuous
     * battery consumption for mobile devices, and playback failure for other
     * applications if no multiple instances of the same codec are supported on
     * a device. Even if multiple instances of the same codec are supported,
     * some performance degradation may be expected when unnecessary multiple
     * instances are used at the same time.
     */
    public void release() {
        mOnDMRRequestEvent = null;
        native_release();
    }

    private native void native_release();

    /**
     * Sets the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to be set.
     * @param value
     *            value of the parameter to be set.
     * @return true if the parameter is set successfully, false otherwise
     *         {@hide
   * }
     */
    private native boolean native_setParameter(int key, Parcel value);

    /**
     * Sets the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to be set.
     * @param value
     *            value of the parameter to be set.
     * @return true if the parameter is set successfully, false otherwise
     *         {@hide
   * }
     */
    public boolean setParameter(int key, String value) {
        Parcel p = Parcel.obtain();
        p.writeString(value);
        boolean ret = native_setParameter(key, p);
        p.recycle();
        return ret;
    }

    /**
     * Sets the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to be set.
     * @param value
     *            value of the parameter to be set.
     * @return true if the parameter is set successfully, false otherwise
     *         {@hide
   * }
     */
    public boolean setParameter(int key, int value) {
        Parcel p = Parcel.obtain();
        p.writeInt(value);
        boolean ret = native_setParameter(key, p);
        p.recycle();
        return ret;
    }

    /**
     * Gets the value of the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to get.
     * @param reply
     *            value of the parameter to get.
     */
    private native void native_getParameter(int key, Parcel reply);

    /**
     * Gets the value of the parameter indicated by key. The caller is
     * responsible for recycling the returned parcel.
     * 
     * @param key
     *            key indicates the parameter to get.
     * @return value of the parameter. {@hide}
     */
    public Parcel getParcelParameter(int key) {
        Parcel p = Parcel.obtain();
        native_getParameter(key, p);
        return p;
    }

    /**
     * Gets the value of the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to get.
     * @return value of the parameter. {@hide}
     */
    public String getStringParameter(int key) {
        Parcel p = Parcel.obtain();
        native_getParameter(key, p);
        String ret = p.readString();
        p.recycle();
        return ret;
    }

    /**
     * Gets the value of the parameter indicated by key.
     * 
     * @param key
     *            key indicates the parameter to get.
     * @return value of the parameter. {@hide}
     */
    public int getIntParameter(int key) {
        Parcel p = Parcel.obtain();
        native_getParameter(key, p);
        int ret = p.readInt();
        p.recycle();
        return ret;
    }

    /**
     * @param request
     *            Parcel destinated to the media player. The Interface token
     *            must be set to the IMediaPlayer one to be routed correctly
     *            through the system.
     * @param reply
     *            [out] Parcel that will contain the reply.
     * @return The status code.
     */
    private native int native_invoke(Parcel request, Parcel reply);

    private static native void native_init();

    private native void native_setup(Object DMRDevice_this, String root_path, String dmrName);

    private native void native_finalize();

    @Override
    protected void finalize() {
        Log.d(TAG, "DMRClient to finalize...DMRDeviceClient.this: "
                + DMRDeviceClient.this);
        native_finalize();
    }

    /*
     * Do not change these values without updating their counterparts in
     * rhino/dlna/android/libdlna/IDLNAService.h!
     */
    private static final int EVENT_DLNA_NOP = 0; // interface test message
    private static final int EVENT_DLNA_DMP_EVENT_START = 1;
    private static final int EVENT_DLNA_DMSLIST_CHANGED = 2;
    private static final int EVENT_DLNA_FILELIST_CHANGE = 3;
    private static final int EVENT_DLNA_CHANNEL_RESOURCE_CHANGED = 4;
    private static final int EVENT_DLNA_DMP_EVENT_END = 99;

    private static final int EVENT_DLNA_DMR_EVENT_START = 100;
    private static final int EVENT_DLNA_DMR_EVENT_END = 199;

    public static final int EVENT_DLNA_ERROR = 400;
    private static final int EVENT_DLNA_INFO = 500;

    // DLNA ERROR TYPE
    public static final int DLNA_ERROR_UNKNOWN = 1;
    // 1xx
    public static final int DLNA_ERROR_SERVER_DIED = 100;
    // 2xx
    public static final int DLNA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200;

    private static final String DEFAULT_JSON = "{\"returncode\": \"0\"}";

    private class EventHandler extends Handler {
        private DMRDeviceClient m_dmp;

        public EventHandler(DMRDeviceClient dmp, Looper looper) {
            super(looper);
            m_dmp = dmp;
        }

        @Override
        public void handleMessage(Message msg) {
            String value;
            if (m_dmp.mNativeDMRDeviceClient == 0) {
                Log.w(TAG, "dmr device went away with unhandled events");
                return;
            }
            if ((msg.obj instanceof String)) {
                Log.w(TAG,
                        "msg.obj is String object. value=" + msg.obj.toString());
                value = (String) msg.obj;
            } else {
                value = msg.obj.toString();
            }
            Log.d(TAG,
                    "message type " + msg.what + " msg.obj="
                            + msg.obj.toString());
            String result = mOnDMRRequestEvent.onEvent(m_dmp, msg.what,
                    msg.arg1, msg.arg2, value);
        }
    }

    /**
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original MediaPlayer object so
     * that the native code is safe from the object disappearing from underneath
     * it. (This is the cookie passed to native_setup().)
     */
    @CalledByNative
    private static void postEventFromNative(Object dmr_device_ref, int what,
            int arg1, int arg2, String obj) {
        DMRDeviceClient mp = (DMRDeviceClient) ((WeakReference<?>) dmr_device_ref)
                .get();
        if (mp == null) {
            return;
        }

        if (mp.mEventHandler != null) {
            Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            mp.mEventHandler.sendMessage(m);
        }
    }

    /**
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original MediaPlayer object so
     * that the native code is safe from the object disappearing from underneath
     * it. (This is the cookie passed to native_setup().)
     */
    @CalledByNative
    private static String postEventFromNativeSync(Object dmr_device_ref,
            int what, int arg1, int arg2, String obj) {
        DMRDeviceClient mp = (DMRDeviceClient) ((WeakReference<?>) dmr_device_ref)
                .get();
        if (mp == null) {
            return DEFAULT_JSON;
        }
        return mp.HandleEventSync(what, arg1, arg2, obj);

    }

    private String HandleEventSync(int what, int arg1, int arg2, String obj) {
        Log.d(TAG, "HandleEventSync-----------DMRDeviceClient.this: "
                + DMRDeviceClient.this);
        String value = "null";
        if (obj != null) {
            if ((obj instanceof String)) {
                Log.w(TAG, "obj is String object. value=" + obj.toString());
                value = (String) obj;
            } else {
                value = obj.toString();
            }
        }
        Log.d(TAG, "message type " + what + " msg.obj=" + value);
        switch (what) {
        case EVENT_DLNA_ERROR:
            if (DLNA_ERROR_SERVER_DIED == arg1) {
                Log.i(TAG, "DLNA service died!");
            }
            break;
        default:
            break;
        }
        if (null != mOnDMRRequestEvent)
            return mOnDMRRequestEvent.onEvent(this, what, arg1, arg2, value);
        return DEFAULT_JSON;
    }

    private OnDMRRequestEvent mOnDMRRequestEvent;

    /**
     * Interface definition for a callback to be invoked when playback of a
     * media source has completed.
     */
    public interface OnDMRRequestEvent {
        /**
         * Called when the end of a media source is reached during playback.
         * 
         * @param mp
         *            the MediaPlayer that reached the end of the file
         */
        String onEvent(DMRDeviceClient mp, int what, int arg1, int arg2,
                String obj);
    }

    /**
     * Register a callback to be invoked when the end of a media source has been
     * reached during playback.
     * 
     * @param listener
     *            the callback that will be run
     */
    public void setDMRRequestEvent(OnDMRRequestEvent listener) {
        mOnDMRRequestEvent = listener;
    }

    private native int native_setResult(Parcel result);

    public void setResult(String value) {
        Parcel result = Parcel.obtain();
        result.writeString(value);
        native_setResult(result);
        result.recycle();
        return;
    }
}
