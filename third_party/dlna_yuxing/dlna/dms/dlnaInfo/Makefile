#DLNA_PROJECT_PATH = $(shell pwd)/../../../..
DLNA_PROJECT_PATH = ../../..
COMMON_PATH = $(DLNA_PROJECT_PATH)/modules/common.path
COMMON_MAK = $(DLNA_PROJECT_PATH)/modules/common.mak

ifeq (,$(wildcard $(COMMON_PATH)))
$(error "config.path is not present!!!!")
endif
include $(COMMON_PATH)

ifeq (,$(wildcard config.mak))
$(error "config.mak is not present, run configure !")
endif
include config.mak

DISTFILE = libdlna-$(VERSION).tar.bz2
PKGCONFIG_DIR = $(libdir)/pkgconfig
PKGCONFIG_FILE = libdlna.pc

LIBTEST = test-libdlna
SRCS = test-libdlna.c

EXTRADIST = AUTHORS \
	ChangeLog \
	configure \
	COPYING \
	README \

SUBDIRS = src \

CFLAGS += -Isrc -D_XOPEN_SOURCE=600 -I$(INCLUDES_PATH)
LDFLAGS += -Lsrc 

SHARED_PATH = $(SHARED_LIBS_PATH)
#LDFLAGS = /home/yanyongmeng/dmp/libdlna-0.2.3/src/libdlna.a $(libavformatdir)/libavformat.a $(libavcodecdir)/libavcodec.a $(libavutildir)/libavutil.a -lm
#LDFLAGS = $(SHARED_PATH)/libavutil.so.50 $(SHARED_PATH)/libavcodec.so.52 $(SHARED_PATH)/libavformat.so.52 $(SHARED_PATH)/libdlna.so.0.2.3 -lm
LDFLAGS := -L$(SHARED_PATH) -lavutil -lavcodec -lavformat -ldlnaInfo -lm

ifeq ($(BUILD_STATIC),yes)
  LDFLAGS += $(EXTRALIBS)
endif

all: lib test

lib:
	$(MAKE) -C src

test: $(LIBTEST)

$(LIBTEST): $(SRCS)
	$(CC) $? $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	$(MAKE) -C src clean
	-$(RM) -f $(LIBTEST)

distclean: clean
	-$(RM) -f config.log
	-$(RM) -f config.mak
	-$(RM) -f $(PKGCONFIG_FILE)

install: install-pkgconfig
	$(MAKE) -C src install

install-pkgconfig: $(PKGCONFIG_FILE)
	$(INSTALL) -d "$(PKGCONFIG_DIR)"
	$(INSTALL) -m 644 $< "$(PKGCONFIG_DIR)"

.PHONY: clean distclean
.PHONY: install install-pkgconfig

dist:
	-$(RM) $(DISTFILE)
	dist=$(shell pwd)/libdlna-$(VERSION) && \
	for subdir in . $(SUBDIRS); do \
		mkdir -p "$$dist/$$subdir"; \
		$(MAKE) -C $$subdir dist-all DIST="$$dist/$$subdir"; \
	done && \
	tar cjf $(DISTFILE) libdlna-$(VERSION)
	-$(RM) -rf libdlna-$(VERSION)

dist-all:
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist dist-all
