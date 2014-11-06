#DLNA_PROJECT_PATH = $(shell pwd)/../../../..
DLNA_PROJECT_PATH = ../../../..
COMMON_PATH = $(DLNA_PROJECT_PATH)/modules/common.path
COMMON_MAK = $(DLNA_PROJECT_PATH)/modules/common.mak

ifeq (,$(wildcard $(COMMON_PATH)))
$(error "config.path is not present!!!!")
endif
include $(COMMON_PATH)

ifeq (,$(wildcard ../config.mak))
$(error "../config.mak is not present, run configure !")
endif
include ../config.mak

ifeq ($(BUILD_STATIC),yes)
  BUILD_RULES   += lib_static
  INSTALL_RULES += install_static
endif
ifeq ($(BUILD_SHARED),yes)
  BUILD_RULES   += lib_shared
  INSTALL_RULES += install_shared
endif

LIBNAME = libdlnaInfo
LIBNAME_SHARED = ${LIBNAME}.so
LIBNAME_STATIC = ${LIBNAME}.a
LIBNAME_VERSION = $(LIBNAME_SHARED).$(VERSION)
LIBNAME_MAJOR = $(LIBNAME_SHARED).$(shell echo $(VERSION) | cut -f1 -d.)

SRCS =  profiles.c \
	containers.c \
	image_jpeg.c \
	image_png.c \
	audio_aac.c \
	audio_ac3.c \
	audio_amr.c \
	audio_atrac3.c \
	audio_g726.c \
	audio_lpcm.c \
	audio_mp2.c \
	audio_mp3.c \
	audio_wma.c \
	av_mpeg1.c \
	av_mpeg2.c \
	av_mpeg4_part2.c \
	av_mpeg4_part10.c \
	av_wmv9.c \
	upnp_dms.c \

EXTRADIST = dlna.h \
	dlna_internals.h \
	containers.h \
	profiles.h \
	
#CFLAGS = -I..
FFMPEG_PATH = /mnt/data/coding/dmp/ffmpeg-export-2010-04-13
CFLAGS = -I$(FFMPEG_PATH) -I$(FFMPEG_PATH)/libavcodec -I$(FFMPEG_PATH)/libavutil -I$(FFMPEG_PATH)/libavformat -I$(INCLUDES_PATH)

ifeq ($(DEBUG),yes)
CFLAGS += -DHAVE_DEBUG 
endif

OBJS = $(SRCS:.c=.o)
LOBJS = $(SRCS:.c=.lo)

.SUFFIXES: .c .o .lo

all: depend $(BUILD_RULES)

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) -o $@ $<

.c.lo:
	$(CC) -c -fPIC $(CFLAGS) $(OPTFLAGS) -o $@ $<

lib_static_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building static objects           #"
	@echo "#############################################"

lib_static_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking static objects            #"
	@echo "#############################################"

lib_static: lib_static_info_pre $(OBJS) lib_static_info_post
	$(AR) r $(LIBNAME_STATIC) $(OBJS)
	$(RANLIB) $(LIBNAME_STATIC)

lib_shared_info_pre:
	@echo 
	@echo "#############################################"
	@echo "#         Building shared objects           #"
	@echo "#############################################"

lib_shared_info_post:
	@echo 
	@echo "#############################################"
	@echo "#         Linking shared objects            #"
	@echo "#############################################"

lib_shared: lib_shared_info_pre $(LOBJS) lib_shared_info_post
	$(CC) -shared $(LOBJS) \
	  $(LDFLAGS) $(EXTRALIBS) -o $(LIBNAME_SHARED)
		cp $(LIBNAME_SHARED) $(SHARED_LIBS_PATH)/
		cp $(LIBNAME_SHARED) $(ROOTFS_SHARED_PATH)/	
		cp $(LIBNAME_SHARED) $(TRUNK_SHARED_PATH)/	
#	$(CC) -shared -Wl,-soname,$(LIBNAME_MAJOR) $(LOBJS) \
#	  $(LDFLAGS) $(EXTRALIBS) -o $(LIBNAME_VERSION)
#	$(LN) -sf $(LIBNAME_VERSION) $(LIBNAME_MAJOR)
#	$(LN) -sf $(LIBNAME_MAJOR) $(LIBNAME_SHARED)

clean:
	-$(RM) -f *.o *.lo *.a *.so*
	-$(RM) -f .depend

install_static: lib_static
	$(INSTALL) -d $(libdir)
	$(INSTALL) $(LIBNAME_STATIC) $(libdir)

install_shared: lib_shared
	$(INSTALL) -d $(libdir)
	$(INSTALL) $(LIBNAME_VERSION) $(libdir)
	$(LN) -sf $(LIBNAME_VERSION) $(libdir)/$(LIBNAME_MAJOR)
	$(LN) -sf $(LIBNAME_MAJOR) $(libdir)/$(LIBNAME_SHARED)

install: $(INSTALL_RULES)
	$(INSTALL) -d $(includedir)
	$(INSTALL) -m 644 dlna.h $(includedir)

depend:
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

.PHONY: clean depend

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist-all

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
