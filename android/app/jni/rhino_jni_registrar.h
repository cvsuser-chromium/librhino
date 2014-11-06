// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DLNA_ANDROID_RHINO_JNI_REGISTRAR_H_
#define DLNA_ANDROID_RHINO_JNI_REGISTRAR_H_

#include <jni.h>

bool RegisterDlnaDMRDeviceClient(JNIEnv* env);
bool RegisterDlnaDMPDevice(JNIEnv* env);
bool RegisterAirPlayClient(JNIEnv* env);

#endif  // BASE_ANDROID_BASE_JNI_REGISTRAR_H_
