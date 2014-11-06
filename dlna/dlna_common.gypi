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
      'target_name': 'dlna_impl',
      'type': '<(component)',
      'dependencies': [
         '<(DEPTH)/base/base.gyp:base',
         '<(DEPTH)/ipc/ipc.gyp:ipc',
         '<(DEPTH)/rhino/third_party/dlna_yuxing/yuxing_dlna.gyp:yuxing_dlna_impl',
         '<(DEPTH)/rhino/third_party/libupnp-1.6.18/libupnp.gyp:libupnp',
      ],
      'defines': [
      ],
      'include_dirs': [
        '../include',
        'common',
      ],
      'direct_dependent_settings': {
        'defines': [
        ],
        'include_dirs': [
        ],
        'linkflags': [
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [              
      ],
      'conditions': [
        ['OS=="linux"', {
            'defines': [
                'LINUX_DEFINE',
            ],
            'include_dirs': [
                'include/linux',
            ],
            'cflags': [
              '-g',
              '-O2',
              '-Wall',
            ]
        }],
        ['OS=="android"', {
          'includes': [
              '../android/android_common.gypi',
          ],
          'dependencies': [
          ],
          'include_dirs': [
              'android/libdlna',
          ],
          'cflags': [
              '<@(android_system_headers)',
          ],
          'sources': [ 
              'android/libdlna/dlna_common.cc',
              'android/libdlna/dmp_device.cc',
              'android/libdlna/dmr_device_client.cc',            
          ],
        }],
        ['OS=="win"', {
          'defines': [
            'OS_WIN32',
          ],
          'include_dirs': [
          ],
          'cflags': [
            '/W3',
            '/wd4996',
            '/nologo',
          ]
        }, { # OS != "win",
          'defines': [
            'NON_WINDOWS_DEFINE',
          ],
        }]
      ]
    },
  ],
}

