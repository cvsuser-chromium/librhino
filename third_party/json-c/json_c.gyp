# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'json_c',
      'type': '<(component)',
      'dependencies': [        
        '<(DEPTH)/base/base.gyp:base',
      ],
      'defines': [
        'HAVE_CONFIG_H',
				'_FILE_OFFSET_BITS=64',
				'_XOPEN_SOURCE=500',
      ],
      'include_dirs': [
        '.',
      ],
      'direct_dependent_settings': {
        'defines': [
        ],
        'include_dirs': [
          '.',
          'json',
        ],
        'linkflags': [
        ],
      },
      'export_dependent_settings': [
        #'<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        'json/linkhash.cc', 
        'json/json_util.c',
        'json/debug.c',
        'json/json_tokener.c',
        'json/printbuf.c',
        'json/json_public.c',
        'json/json_object.c',
        'json/arraylist.c',
        'json/json_object_iterator.c',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            #'OS_LINUX',
          ],
          'include_dirs': [
            'include/linux',
          ],
        }],
        ['OS=="android"', {
          'dependencies': [
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
            #'NON_WINDOWS_DEFINE',
          ],
        }],
      ],
    },
  ],
}

