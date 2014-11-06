# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'configurations': {
        'Debug': {
          'defines': [
            'DEBUG',
            'DLNA_ENABLE_LOG',
          ],
        },
        'Release': {
        },
      },
      'target_name': 'libdlna_jni',
      'type': 'shared_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'dlna_jni_headers',
        '<(DEPTH)/rhino/third_party/dlna_yuxing/yuxing_dlna.gyp:yuxing_dlna_impl',
        '<(DEPTH)/rhino/third_party/libupnp-1.6.18/libupnp.gyp:libupnp',
      ],
      'defines': [
        
      ],
      'include_dirs': [
        'android',
        '../include',
      ],
      'cflags': [
      ],
      'ldflags': [
      ],
      'direct_dependent_settings': {
        'defines': [
        ],
        'include_dirs': [
          '.',
        ],
      },
      'sources': [
      ],
      'conditions': [
        
      ]
    },
    {
      'target_name': 'dlna_jni_headers',
      'type': 'none',
      'dependencies': [
        #'java_set_jni_headers',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/dlna',
        ],
      },
    },
  ]
}

