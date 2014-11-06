# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    'dlna_common.gypi',
  ],
  'conditions': [
    ['OS=="linux"', {
    }],
    ['OS=="android"', {
      'includes': [
        #'dlna_service.gypi',
      ],
    }],
  ],
  'targets': [
   {
     'target_name': 'dlna',
     'type': 'none',
     'dependencies': [
       'dlna_impl',
     ],
     'conditions': [
       ['OS=="linux"', {
         'dependencies': [
           'dlna_impl',
         ],
       }],
       ['OS=="android"', {
         'dependencies': [
           #'dlna_impl',
        ],
       }],
       ['OS=="win"', {
         'defines': [
           'OS_WIN32',
         ],
         'dependencies': [
           'dlna_impl',
           #'dlnaserver.elf',
         ],
         'include_dirs': [
         ],
         'cflags': [
           '/W3',
           '/wd4996',
           '/nologo',
         ],
       }],
     ],
   }
  ]
}

