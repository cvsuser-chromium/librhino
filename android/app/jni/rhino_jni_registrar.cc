// ----------------------------------------------------------------------------
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/debug/debugger.h"
#include "base/logging.h"
#include "base/android/jni_android.h"
#include "base/at_exit.h"

#include "rhino_jni_registrar.h"

namespace {
base::AtExitManager* g_at_exit_manager = NULL;
}

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();

  // We need the AtExitManager to be created at the very beginning.
  g_at_exit_manager = new base::AtExitManager();

  if (!RegisterDlnaDMPDevice(env))
    return -1;

  if (!RegisterDlnaDMRDeviceClient(env))
    return -1;
  
  if (!RegisterAirPlayClient(env))
    return -1;
  LOG(INFO) << "librhino_jni Loaded!";
  return JNI_VERSION_1_4;
}
JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
  delete g_at_exit_manager;
  LOG(INFO) << "librhino_jni Unloaded!";
  base::android::DetachFromVM();
}
