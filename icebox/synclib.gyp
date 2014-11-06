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
      'target_name': 'synclib',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/url/url.gyp:url_lib',        
        '<(DEPTH)/third_party/libjingle/libjingle.gyp:libjingle',
      ],
      'defines': [
      ],
      'include_dirs': [
        '.',
        '<(DEPTH)/testing/gtest/include',
      ],
      'direct_dependent_settings': {
        'defines': [
        ],
        'include_dirs': [
          '.',
        ],
        'linkflags': [
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        'synclib_main.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            'TARGET_POSIX',
            'TARGET_LINUX',
            'HAS_LINUX_NETWORK',
          ],
          'dependencies': [
          ],
          'include_dirs': [
          ],
          'sources': [
          ],
          'cflags': [
            '-g',
            '-O2',
            '-Wall',
            '-fexceptions',
          ],          
        }],
        ['OS=="android"', {
          'defines': [
            'TARGET_POSIX',
            'TARGET_ANDROID',
            'HAS_LINUX_NETWORK',
          ],
          'dependencies': [
          ],
          'include_dirs': [
          ],
          'sources': [
          ],
          'cflags': [
            '-g',
            '-O2',
            '-Wall',
#            '-fexceptions',
          ],
          'direct_dependent_settings': {
            'defines': [
              'TARGET_POSIX',
              'TARGET_ANDROID',
            ],
            'include_dirs': [
            ],
          },
        }],
        ['OS=="win"', {
          'defines': [
            'TARGET_WINDOWS',
            '_VC80_UPGRADE=0x0710',
            '_MBCS',
            '_WINDOWS',
            '_MSVC',
            'WIN32',
            '_WIN32_WINNT=0x0600',
            'NTDDI_VERSION=0x06000000',
            'NOMINMAX',
            '_USE_32BIT_TIME_T',
            'HAS_DX',
            'D3D_DEBUG_INFO',
            '__STDC_CONSTANT_MACROS',
            'TAGLIB_STATIC',
            'HAS_WIN32_NETWORK',
          ],
          'include_dirs': [
          ],
          'direct_dependent_settings': {
            'defines': [
              'TARGET_WINDOWS',
              '_VC80_UPGRADE=0x0710',
              '_MBCS',
              '_WINDOWS',
              '_MSVC',
              'WIN32',
              '_WIN32_WINNT=0x0600',
              'NTDDI_VERSION=0x06000000',
              'NOMINMAX',
              '_USE_32BIT_TIME_T',
              'HAS_DX',
              'D3D_DEBUG_INFO',
              '__STDC_CONSTANT_MACROS',
              #'_HAS_ITERATOR_DEBUGGING=0',
              'TAGLIB_STATIC',
              'HAS_WIN32_NETWORK',
            ],
            'include_dirs': [
              '.',
              '<(DEPTH)/rhino/airplay/win32',
            ],
            'linkflags': [
            ],
          },
          'cflags': [
            '/W3',
            '/wd4996',
            '/nologo',
          ],
          'sources': [
          ],
        }],
      ],
    },
  ],
}

