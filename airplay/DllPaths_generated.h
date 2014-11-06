#ifndef DLL_PATHS_GENERATED_H_
#define DLL_PATHS_GENERATED_H_

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

/* prefix install location */
#define PREFIX_USR_PATH        "/home/reviewboard/tmp/xbmc-deps/arm-linux-androideabi-android-14"

/* libraries */
#define DLL_PATH_CPLUFF        "special://xbmcbin/system/libcpluff-arm.so"
#define DLL_PATH_IMAGELIB      "special://xbmcbin/system/ImageLib-arm.so"
#define DLL_PATH_LIBEXIF       "special://xbmcbin/system/libexif-arm.so"
#define DLL_PATH_LIBHDHOMERUN  "special://xbmcbin/system/hdhomerun-arm.so"
#define DLL_PATH_MEDIAINFO     "special://xbmcbin/system/mediainfo-arm.so"
#define DLL_PATH_LIBCMYTH      "special://xbmcbin/system/libcmyth-arm.so"

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
#define DLL_PATH_AAC_CODEC     "special://xbmcbin/system/players/paplayer/AACCodec-arm.so"
#define DLL_PATH_ADPCM_CODEC   "special://xbmcbin/system/players/paplayer/adpcm-arm.so"
#define DLL_PATH_ADPLUG_CODEC  "special://xbmcbin/system/players/paplayer/adplug-arm.so"
#define DLL_PATH_APE_CODEC     "special://xbmcbin/system/players/paplayer/MACDll-arm.so"
#define DLL_PATH_ASAP_CODEC    "special://xbmcbin/system/players/paplayer/xbmc_asap-arm.so"
#define DLL_PATH_DCA_CODEC     "special://xbmcbin/system/players/paplayer/dcacodec-arm.so"
#define DLL_PATH_GYM_CODEC     "special://xbmcbin/system/players/paplayer/gensapu-arm.so"
#define DLL_PATH_MID_CODEC     "special://xbmcbin/system/players/paplayer/timidity-arm.so"
#define DLL_PATH_MODULE_CODEC  "special://xbmcbin/system/players/paplayer/dumb-arm.so"
#define DLL_PATH_MPC_CODEC     "special://xbmcbin/system/players/paplayer/libmpcdec-arm.so"
#define DLL_PATH_NSF_CODEC     "special://xbmcbin/system/players/paplayer/nosefart-arm.so"
#define DLL_PATH_SID_CODEC     "special://xbmcbin/system/players/paplayer/libsidplay2-arm.so"
#define DLL_PATH_SPC_CODEC     "special://xbmcbin/system/players/paplayer/SNESAPU-arm.so"
#define DLL_PATH_VGM_CODEC     "special://xbmcbin/system/players/paplayer/vgmstream-arm.so"
#define DLL_PATH_WAVPACK_CODEC "@WAVPACK_SONAME@"
#define DLL_PATH_YM_CODEC      "special://xbmcbin/system/players/paplayer/stsoundlibrary-arm.so"

#define DLL_PATH_FLAC_CODEC    "libxbFLAC.so"
#define DLL_PATH_MODPLUG_CODEC "libmodplug.so"
#define DLL_PATH_OGG_CODEC     "libxbvorbisfile.so"

/* dvdplayer */
#define DLL_PATH_LIBASS        "libass.so"
#define DLL_PATH_LIBDVDNAV     "special://xbmcbin/system/players/dvdplayer/libdvdnav-arm.so"
#define DLL_PATH_LIBMPEG2      "libxbmpeg2.so"
#define DLL_PATH_LIBMAD        "libmad.so"

/* ffmpeg */
#define DLL_PATH_LIBAVCODEC    "special://xbmcbin/system/players/dvdplayer/avcodec-54-arm.so"
#define DLL_PATH_LIBAVFORMAT   "special://xbmcbin/system/players/dvdplayer/avformat-54-arm.so"
#define DLL_PATH_LIBAVUTIL     "special://xbmcbin/system/players/dvdplayer/avutil-52-arm.so"
#define DLL_PATH_LIBPOSTPROC   "special://xbmcbin/system/players/dvdplayer/postproc-52-arm.so"
#define DLL_PATH_LIBSWSCALE    "special://xbmcbin/system/players/dvdplayer/swscale-2-arm.so"
#define DLL_PATH_LIBAVFILTER   "special://xbmcbin/system/players/dvdplayer/avfilter-3-arm.so"
#define DLL_PATH_LIBSWRESAMPLE "special://xbmcbin/system/players/dvdplayer/swresample-0-arm.so"

/* cdrip */
#define DLL_PATH_LAME_ENC      "libmp3lame.so"
#define DLL_PATH_OGG           "libxbogg.so"
#define DLL_PATH_VORBIS_ENC    "libxbvorbisenc.so"
#define DLL_PATH_VORBIS        "libxbvorbis.so"

/* broadcom crystalhd */
#if defined(TARGET_DARWIN)
#define DLL_PATH_LIBCRYSTALHD  "libcrystalhd.dylib"
#else
#define DLL_PATH_LIBCRYSTALHD  ""
#endif

/* libbluray */
#define DLL_PATH_LIBBLURAY     "libbluray.so"

#endif
