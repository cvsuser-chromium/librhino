# Makefile	makefile for the East Fork based on sunplus PHE1001A
#
# Copyright (c) 1998-2004 Gold Yuxing, Inc.
#
# Description
# ----------------
# 1. This makefile specially for building EF .bin image on the sunplus's SPHE1001 platform.
# 2. Based on Intel's UPnP MicroStack/1.0 by Intel Device Builder - Build#1.0.1068.25676
#
# Revision history
# Dec.30,2004  Valiant (yanyongmeng@yu-xing.com):
#  o Created 
# Mar.25,2005  Valiant (yanyongmeng@yu-xing.com):
#  o intergrated 
#
#-----------------------------------------------------------------------
# basic define for tools
#-----------------------------------------------------------------------
BUILDPATH=/home/wangdedong/st/stdma/4_build
PRJ_DIR		= ../..
MAKEFILERULESPATH := $(BUILDPATH)/makefile.rules
include $(MAKEFILERULESPATH)/Makefile.rules

# Name of the lib
LIBNAME				:= lib_$(PRELIBNAME)_$(PROVIDER)_$(VERSION)_$(SUPPORT_TYPE).a
DEFINE += -DLIB_DBG_ENABLE=DBG_$(PRE2LIBNAME)_$(PRELIBNAME)

#-----------------------------------------------------------------------
${LIBNAME} : $(OBJFILES)
	$(AR) $(AFLAGS) $(LIBNAME) $(OBJFILES)
	$(STRIP) ${LIBNAME}
	cp ${LIBNAME} $(HEADFILEPATH)/1_H/${LIBPATH}/lib

%.o:%.c
	$(CC) -c -o $@ $< $(CFLAGS)
		
clean:
	rm $(OBJFILES) ${LIBNAME}
#-----------------------------------------------------------------------
	
