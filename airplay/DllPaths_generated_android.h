/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once
/* libraries */

//Android rules:
// - All solibs must be in the form ^lib*.so$
// - All solibs must be in the same dir
// - Arch need not be specified, each arch will get its own lib dir.
//   We only keep arm here to retain the same structure as *nix.
// * foo.so will be renamed libfoo.so in the packaging stage

#define DLL_PATH_CPLUFF        "libcpluff-arm.so"
#define DLL_PATH_IMAGELIB      "libImageLib-arm.so"
#define DLL_PATH_LIBEXIF       "libexif-arm.so"
#define DLL_PATH_LIBHDHOMERUN  "libhdhomerun-arm.so"
#define DLL_PATH_MEDIAINFO     "libmediainfo-arm.so"
#define DLL_PATH_LIBCMYTH      "libcmyth-arm.so"

#define DLL_PATH_LIBRTMP       "librtmp.so"
#define DLL_PATH_LIBNFS        "libnfs.so"
#define DLL_PATH_LIBAFP        "libafpclient.so"
#define DLL_PATH_LIBPLIST      "libplist.so"
#define DLL_PATH_LIBSHAIRPORT  ""
#define DLL_PATH_LIBSHAIRPLAY  "libshairplay.so"
#define DLL_PATH_LIBCEC        "libcec.so"

#ifndef DLL_PATH_LIBCURL
#define DLL_PATH_LIBCURL       "libcurl.so"
#endif

/* paplayer */
#define DLL_PATH_AAC_CODEC     "libAACCodec-arm.so"
#define DLL_PATH_ADPCM_CODEC   "libadpcm-arm.so"
#define DLL_PATH_ADPLUG_CODEC  "libadplug-arm.so"
#define DLL_PATH_APE_CODEC     "libMACDll-arm.so"
#define DLL_PATH_ASAP_CODEC    "libxbmc_asap-arm.so"
#define DLL_PATH_DCA_CODEC     "libdcacodec-arm.so"
#define DLL_PATH_GYM_CODEC     "libgensapu-arm.so"
#define DLL_PATH_MID_CODEC     "libtimidity-arm.so"
#define DLL_PATH_MODULE_CODEC  "libdumb-arm.so"
#define DLL_PATH_MPC_CODEC     "libmpcdec-arm.so"
#define DLL_PATH_NSF_CODEC     "libnosefart-arm.so"
#define DLL_PATH_SID_CODEC     "libsidplay2-arm.so"
#define DLL_PATH_SPC_CODEC     "libSNESAPU-arm.so"
#define DLL_PATH_VGM_CODEC     "libvgmstream-arm.so"
#define DLL_PATH_WAVPACK_CODEC "@WAVPACK_SONAME@"
#define DLL_PATH_YM_CODEC      "libstsoundlibrary-arm.so"

#define DLL_PATH_FLAC_CODEC    "libxbFLAC.so"
#define DLL_PATH_MODPLUG_CODEC "libmodplug.so"
#define DLL_PATH_OGG_CODEC     "libxbvorbisfile.so"

/* dvdplayer */
#define DLL_PATH_LIBASS        "libass.so"
#define DLL_PATH_LIBDVDNAV     "libdvdnav-arm.so"
#define DLL_PATH_LIBMPEG2      "libxbmpeg2.so"
#define DLL_PATH_LIBMAD        "libmad.so"

/* ffmpeg */
#define DLL_PATH_LIBAVCODEC    "libavcodec-54-arm.so"
#define DLL_PATH_LIBAVFORMAT   "libavformat-54-arm.so"
#define DLL_PATH_LIBAVUTIL     "libavutil-52-arm.so"
#define DLL_PATH_LIBPOSTPROC   "libpostproc-52-arm.so"
#define DLL_PATH_LIBSWSCALE    "libswscale-2-arm.so"
#define DLL_PATH_LIBAVFILTER   "libavfilter-3-arm.so"
#define DLL_PATH_LIBSWRESAMPLE "libswresample-0-arm.so"

/* cdrip */
#define DLL_PATH_LAME_ENC      "libmp3lame.so"
#define DLL_PATH_OGG           "libxbogg.so"
#define DLL_PATH_VORBIS_ENC    "libxbvorbisenc.so"
#define DLL_PATH_VORBIS        "libxbvorbis.so"

/* libbluray */
#define DLL_PATH_LIBBLURAY     "libbluray.so"

/* Android's libui for gralloc */
#define DLL_PATH_LIBUI         "libui.so"
