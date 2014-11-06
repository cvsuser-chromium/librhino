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

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.PowerManager;
import android.util.Log;
import java.lang.ref.WeakReference;
//import org.chromium.base.AccessedByNative;
import org.chromium.base.CalledByNative;
//import org.chromium.base.CalledByNativeUnchecked;
//import org.chromium.base.JNINamespace;
//import org.chromium.base.NativeClassQualifiedName;

public class DMPDevice {
    static {
        native_init();
    }

    private final static String TAG = "DMPDevice";
    // Name of the remote interface for the media player. Must be kept
    // in sync with the 2nd parameter of the IMPLEMENT_META_INTERFACE
    // macro invocation in IMediaPlayer.cpp
    private final static String IDMP_DEVICE = "com.hybroad.IDMPDevice";

    // accessed by native methods
    // DMPDevice class lives in C++ side.
    private int mNativeDMPDevice;
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
     * When done with the MediaPlayer, you should call {@link #release()}, to
     * free the resources. If not released, too many MediaPlayer instances may
     * result in an exception.
     * </p>
     */
    private DMPDevice() {

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
        native_setup(new WeakReference<DMPDevice>(this),"/data/data/com.hybroad.media/files/dlna_root", "dinglei_dmr");

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
        parcel.writeInterfaceToken(IDMP_DEVICE);
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
    public static DMPDevice create(Context context) {
        mContext = context;
        try {

            DMPDevice dmp = new DMPDevice();
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
        mOnDMSListChangedListener = null;
        mOnFileListChangedListener = null;
        mOnChannelResourceChangedListener = null;
        native_release();
    }

    private native void native_release();

    /**
     * Sets the player to be looping or non-looping.
     * 
     * @param looping
     *            whether to loop or not
     */
    private native void native_setLooping(boolean looping);

    /**
     * Checks whether the MediaPlayer is looping or non-looping.
     * 
     * @return true if the MediaPlayer is currently looping, false otherwise
     */
    private native boolean native_isLooping();

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
        p.recycle();
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

    private native void native_setup(Object DMPDevice_this,String root_path, String dmrName);
    
    private native void native_finalize();

    @Override
    protected void finalize() {
        native_finalize();
    }

    /*
     * Do not change these values without updating their counterparts in
     * rhino/dlna/android/libdlnaservice/dlna_service.h!
     */
    private static final int EVENT_DLNA_DMSLIST_CHANGED = 2;
    private static final int EVENT_DLNA_FILELIST_CHANGE = 3;
    private static final int EVENT_DLNA_CHANNEL_RESOURCE_CHANGED = 4;

    private class EventHandler extends Handler {
        private DMPDevice m_dmp;

        @SuppressLint("HandlerLeak")
        public EventHandler(DMPDevice dmp, Looper looper) {
            super(looper);
            m_dmp = dmp;
        }

        @Override
        public void handleMessage(Message msg) {
            if (m_dmp.mNativeDMPDevice == 0) {
                Log.w(TAG, "dmp device went away with unhandled events");
                return;
            }

            Log.d(TAG, "message type " + msg.what);
            switch (msg.what) {
            case EVENT_DLNA_DMSLIST_CHANGED:
                if (mOnDMSListChangedListener != null)
                    mOnDMSListChangedListener.onChanged(m_dmp);
                return;

            case EVENT_DLNA_FILELIST_CHANGE:
                if (mOnFileListChangedListener != null)
                    mOnFileListChangedListener.onChanged(m_dmp);
                return;

            case EVENT_DLNA_CHANNEL_RESOURCE_CHANGED:
                if (mOnChannelResourceChangedListener != null)
                    mOnChannelResourceChangedListener
                            .onChanged(m_dmp, msg.arg1);
                return;
            default:
                Log.e(TAG, "Unknown message type " + msg.what);
                return;
            }
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
    private static void postEventFromNative(Object dmp_device_ref, int what,
            int arg1, int arg2, String obj) {
        DMPDevice mp = (DMPDevice) ((WeakReference<?>) dmp_device_ref).get();
        if (mp == null) {
            return;
        }

        if (mp.mEventHandler != null) {
            Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            mp.mEventHandler.sendMessage(m);
        }
    }

    /**
     * Interface definition for a callback to be invoked when the media source
     * is ready for playback.
     */
    public interface OnDMSListChangedListener {
        /**
         * Called when the media file is ready for playback.
         * 
         * @param mp
         *            the MediaPlayer that is ready for playback
         */
        void onChanged(DMPDevice mp);
    }

    /**
     * Register a callback to be invoked when the media source is ready for
     * playback.
     * 
     * @param listener
     *            the callback that will be run
     */
    public void setDMSListChangedListener(OnDMSListChangedListener listener) {
        mOnDMSListChangedListener = listener;
    }

    private OnDMSListChangedListener mOnDMSListChangedListener;

    /**
     * Interface definition for a callback to be invoked when playback of a
     * media source has completed.
     */
    public interface OnFileListChangedListener {
        /**
         * Called when the end of a media source is reached during playback.
         * 
         * @param mp
         *            the MediaPlayer that reached the end of the file
         */
        void onChanged(DMPDevice mp);
    }

    /**
     * Register a callback to be invoked when the end of a media source has been
     * reached during playback.
     * 
     * @param listener
     *            the callback that will be run
     */
    public void setFileListChangedListener(OnFileListChangedListener listener) {
        mOnFileListChangedListener = listener;
    }

    private OnFileListChangedListener mOnFileListChangedListener;

    /**
     * Interface definition of a callback to be invoked indicating buffering
     * status of a media resource being streamed over the network.
     */
    public interface OnChannelResourceChangedListener {
        /**
         * Called to update status in buffering a media stream received through
         * progressive HTTP download. The received buffering percentage
         * indicates how much of the content has been buffered or played. For
         * example a buffering update of 80 percent when half the content has
         * already been played indicates that the next 30 percent of the content
         * to play has been buffered.
         * 
         * @param mp
         *            the MediaPlayer the update pertains to
         * @param percent
         *            the percentage (0-100) of the content that has been
         *            buffered or played thus far
         */
        void onChanged(DMPDevice mp, int percent);
    }

    /**
     * Register a callback to be invoked when the status of a network stream's
     * buffer has changed.
     * 
     * @param listener
     *            the callback that will be run.
     */
    public void setOnChannelResourceChangedListener(
            OnChannelResourceChangedListener listener) {
        mOnChannelResourceChangedListener = listener;
    }

    private OnChannelResourceChangedListener mOnChannelResourceChangedListener;

    /* DMP interface. */
    private native int native_getDmsCount(Parcel request, Parcel reply);

    public int getDmsCount() {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        native_getDmsCount(request, reply);
        reply.readInt();
        int ret = reply.readInt();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_researchDms(Parcel request, Parcel reply);

    public int researchDms(boolean remove_all) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeInt(remove_all ? 1 : 0);
        native_researchDms(request, reply);
        int ret = reply.readInt();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getCount(Parcel request, Parcel reply);

    public int getCount(int listID) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeInt(listID);
        native_getCount(request, reply);
        // ignore status_t;
        reply.readInt();
        int ret = reply.readInt();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getDmsList(Parcel request, Parcel reply);

    public String getDmsList(int index, int count) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();

        request.writeInt(index);
        request.writeInt(count);
        native_getDmsList(request, reply);

        // ignore status_t;
        reply.readInt();
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_openFileListByContainer(Parcel request,
            Parcel reply);

    public String openFileList(String deviceID, String containerID,
            String classID, int sort, int order) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeString(deviceID);
        request.writeString(containerID);
        // request.writeString(classID);
        request.writeInt(sort);
        request.writeInt(order);
        native_openFileListByContainer(request, reply);
        // ignore status_t;
        reply.readInt();
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getList(Parcel request, Parcel reply);

    public String getList(int listID, int index, int count) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeInt(listID);
        request.writeInt(index);
        request.writeInt(count);
        native_getList(request, reply);
        // ignore status_t;
        reply.readInt();
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_openFileListByClassID(Parcel request, Parcel reply);

    public String openFileList(String deviceID, String classID, int sort,
            int order) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeString(deviceID);
        request.writeString(classID);
        request.writeInt(sort);
        request.writeInt(order);
        native_openFileListByClassID(request, reply);
        // ignore status_t;
        reply.readInt();
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getGroupInfo(Parcel request, Parcel reply);

    public String getGroupInfo(String deviceID) {
        Parcel request = newRequest();
        Parcel reply = Parcel.obtain();
        request.writeString(deviceID);
        native_getGroupInfo(request, reply);
        // ignore status_t;
        reply.readInt();

        String ret = reply.readString();

        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getGroupLlist(Parcel request, Parcel reply);

    public String getGroupLlist(String deviceID, String group, int index,
            int count, int sort, int order) {
        Parcel request = newRequest();
        request.writeString(deviceID);
        request.writeString(group);
        request.writeInt(index);
        request.writeInt(count);
        request.writeInt(sort);
        request.writeInt(order);

        Parcel reply = Parcel.obtain();
        native_getGroupLlist(request, reply);
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_getGroupItemList(Parcel request, Parcel reply);

    public String getGroupItemList(String deviceID, String group,
            String groupTitle, int index, int count, int sort, int order) {
        Parcel request = newRequest();
        request.writeString(deviceID);
        request.writeString(group);
        request.writeString(groupTitle);
        request.writeInt(index);
        request.writeInt(count);
        request.writeInt(sort);
        request.writeInt(order);

        Parcel reply = Parcel.obtain();
        native_getGroupItemList(request, reply);
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_openFileList_tunerChannel(Parcel request,
            Parcel reply);

    public String openFileList_tunerChannel(String deviceID, int contentSource,
            String classID) {
        Parcel request = newRequest();
        request.writeString(deviceID);
        request.writeInt(contentSource);
        request.writeString(classID);

        Parcel reply = Parcel.obtain();
        native_openFileList_tunerChannel(request, reply);
        String ret = reply.readString();
        request.recycle();
        reply.recycle();
        return ret;
    }

    private native int native_localDB_deleteByDMS(String deviceID);

    public int localDB_deleteByDMS(String deviceID) {
        return native_localDB_deleteByDMS(deviceID);
    }

    private native int native_localDB_searchByKeyWord(String keyWord,
            String matchRule);

    public int localDB_searchByKeyWord(String keyWord, String matchRule) {
        return native_localDB_searchByKeyWord(keyWord, matchRule);
    }

    private native int native_localDB_getSearchList(int listID, int index,
            int count, int classID);

    public int localDB_getSearchList(int listID, int index, int count,
            int classID) {
        // return localDB_getSearchList(listID, index, count, classID);
        return native_localDB_getSearchList(listID, index, count, classID);
    }
}
