# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'android_system_headers%': [
        '-isystem <(android_source_tree)/system/core/include',
#'-isystem <(android_source_tree)/hardware/libhardware/include',
#'-isystem <(android_source_tree)/hardware/libhardware_legacy/include',
#'-isystem <(android_source_tree)/hardware/ril/include',
#'-isystem <(android_source_tree)/dalvik/libnativehelper/include',
          '-isystem <(android_source_tree)/frameworks/base/include',
#'-isystem <(android_source_tree)/frameworks/base/opengl/include',
          '-isystem <(android_source_tree)/frameworks/native/include',
          '-isystem <(android_source_tree)/frameworks/base/core/jni',
#'-isystem <(android_source_tree)/external/skia/include',
#'-isystem <(android_source_tree)/out/target/product/godbox/obj/include',
#'-isystem <(android_source_tree)/bionic/libc/arch-arm/include',
#'-isystem <(android_source_tree)/bionic/libc/include',
#'-isystem <(android_source_tree)/bionic/libstdc++/include',
#'-isystem <(android_source_tree)/bionic/libc/kernel/common',
#'-isystem <(android_source_tree)/bionic/libc/kernel/arch-arm', 
#'-isystem <(android_source_tree)/bionic/libm/include',
#'-isystem <(android_source_tree)/bionic/libm/include/arm',
#'-isystem <(android_source_tree)/bionic/libthread_db/include',
      ],
      'android_system_libs%': [
          #'-lgabi++',
          #'-lcutils',
          '-lutils',
          '-lbinder',
          '-landroid_runtime',
      ],
  },
}
