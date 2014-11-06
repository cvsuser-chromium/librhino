ifeq (1, 1)
INCLUDES+=-I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Core \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Platinum \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Devices/MediaConnect \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Devices/MediaRenderer \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Devices/MediaServer \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Platinum/Source/Extras \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Neptune/Source/System/Posix \
          -I/home/reviewboard/airplay/xbmc/xbmc-android/lib/libUPnP/Neptune/Source/Core

SRCS= UPnP.cpp \
      UPnPInternal.cpp \
      UPnPPlayer.cpp \
      UPnPRenderer.cpp \
      UPnPServer.cpp \
      UPnPSettings.cpp \

LIB=upnp.a

include /home/reviewboard/airplay/xbmc/xbmc-android/Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))

endif

