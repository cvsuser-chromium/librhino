// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.hybroad.dlna;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.Vector;

import android.util.Log;

//import org.chromium.base.JNINamespace;
//import org.chromium.content.common.CommandLine;
//import org.chromium.content.common.ProcessInitException;
//import org.chromium.content.common.ResultCodes;
//import org.chromium.content.common.TraceEvent;

/**
 * This class provides functionality to load and register the native libraries.
 * Callers are allowed to separate loading the libraries from initializing them.
 * This may be an advantage for Android Webview, where the libraries can be
 * loaded by the zygote process, but then needs per process initialization after
 * the application processes are forked from the zygote process.
 * 
 * The libraries may be loaded and initialized from any thread. Synchronization
 * primitives are used to ensure that overlapping requests from different
 * threads are handled sequentially.
 * 
 * See also content/app/android/library_loader_hooks.cc, which contains the
 * native counterpart to this class.
 */
// @JNINamespace("content")
public class LibraryLoader {
    private static final String TAG = "LibraryLoader";

    // Guards all access to the libraries
    private static final Object sLock = new Object();

    // One-way switch becomes true when the libraries are loaded.
    // private static boolean sLoaded = false;

    // One-way switch becomes true when the libraries are initialized (
    // by calling nativeLibraryLoaded, which forwards to LibraryLoaded(...) in
    // library_loader_hooks.cc).
    private static boolean sInitialized = false;

    /**
     * This method blocks until the library is fully loaded and initialized.
     */
    public static void ensureInitialized() {
        synchronized (sLock) {
            if (sInitialized) {
                // Already initialized, nothing to do.
                return;
            }
            // System.loadLibrary("dlna_jni");
            try {
                System.loadLibrary("rhino_jni");
            } catch (UnsatisfiedLinkError ex) {
                ex.printStackTrace();
                Log.d(TAG, ex.getMessage());
                // for test 20131203
                new LibraryLoader().unloadNativeLibs();
            }
            sInitialized = true;
        }
    }

    private void unloadNativeLibs() {
        try {
            ClassLoader classLoader = this.getClass().getClassLoader();
            Field field = ClassLoader.class.getDeclaredField("nativeLibraries");
            field.setAccessible(true);
            Vector<?> libs = (Vector<?>) field.get(classLoader);
            Iterator<?> it = libs.iterator();
            Object o;
            while (it.hasNext()) {
                o = it.next();
                Method finalize = o.getClass().getDeclaredMethod("finalize",
                        new Class[0]);
                finalize.setAccessible(true);
                finalize.invoke(o, new Object[0]);
            }
        } catch (Exception ex) {
            Log.d(TAG, "unload librhino_jni.so failed.");
            throw new RuntimeException(ex);
        }
    }
}
