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
      },
      'Release': {
      },
    },
    'target_name': 'tinyxml',
    'type': 'static_library',
    'dependencies': [
    ],
    'defines': [
      'TIXML_USE_STL',
    ],
    'include_dirs': [
    ],
    'direct_dependent_settings': {
      'defines': [
        'TIXML_USE_STL',
      ],
      'include_dirs': [
        '.',
      ],
    },
    'sources': [
      'tinystr.cpp',
      'tinyxml.cpp',
      'tinyxmlerror.cpp',
      'tinyxmlparser.cpp',
    ],
    'conditions': [
      ['OS=="linux" or OS=="android"', {
        'defines': [
        ],
        'include_dirs': [
          'include/linux',
        ],
        'cflags': [
          '-g',
          '-O2',
          '-Wall',
          '-Wno-format',
        ]
      }],
      ['OS=="win"', {
        'defines': [
          'WIN32',
          '_LIB',
        ],
        'include_dirs': [
        ],
        'cflags': [
        ],
        'link_settings': {
        },
      }]
    ]
  },
 ]
}
