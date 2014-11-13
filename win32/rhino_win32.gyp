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
          ],
          'msvs_configuration_attributes': {
            #'CharacterSet': '0',
          },
        },
        'Release': {
          'msvs_configuration_attributes': {
            #'CharacterSet': '0',
          },
        },
      },
      'target_name': 'rhino.elf',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        #'<(DEPTH)/rhino/airplay/airplay.gyp:airplay',
        #'<(DEPTH)/rhino/third_party/dlna_yuxing/yuxing_dlna.gyp:yuxing_dlna_impl',
        #'<(DEPTH)/rhino/third_party/libupnp-1.6.18/libupnp.gyp:libupnp',
      ],
      'defines': [
      ],
      'include_dirs': [
        '../include',
      ],
      'cflags': [
      ],
      'ldflags': [
      ],
      'sources': [
        #'./dlna_service.cc',
        #'./dlna_server.cc',
        #'./AirPlayClientWin32.cc',
        './synclib_test.cc'
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalDependencies': [
            'Netapi32.lib',
          ],
        },
      },
    },
  ],
}

