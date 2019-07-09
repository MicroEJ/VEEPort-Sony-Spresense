############################################################################
# microjvm/Makefile
#
#   Copyright (C) 2008, 2010-2013 Gregory Nutt. All rights reserved.
#   This file has been modified by MicroEJ Corp (2019).
#
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/.config
-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs
-include $(APPDIR)/Make.defs
include $(SDKDIR)$(DELIM)..$(DELIM)MicroEJ$(DELIM)microej.mk

CONFIG_MICROJVM_TASK_PRIORITY? = SCHED_PRIORITY_DEFAULT
CONFIG_MICROJVM_STACKSIZE? = 4096 

APPNAME = microjvm
PRIORITY = $(CONFIG_MICROJVM_TASK_PRIORITY)
STACKSIZE = $(CONFIG_MICROJVM_STACKSIZE)


CFLAGS += $(MICROEJ_FLAGS)
CFLAGS += -I core/inc/
CFLAGS += -I fs/inc/
CFLAGS += -I ui/inc/
CFLAGS += -I osal/inc/
CFLAGS += -I microej_list/inc/
CFLAGS += -I microej_async_worker/inc/

ifeq ($(CONFIG_MICROEJ_GNSS),y)
CFLAGS += -I gnss/inc/
CSRCS	+= $(wildcard gnss/src/*.c)
endif

CSRCS	+= $(wildcard core/src/*.c)
CSRCS 	+= $(wildcard ui/src/*.c)
CSRCS 	+= $(wildcard fs/src/*.c)
CSRCS 	+= $(wildcard osal/src/*.c)
CSRCS 	+= $(wildcard microej_list/src/*.c)
CSRCS 	+= $(wildcard microej_async_worker/src/*.c)


ifeq ($(CONFIG_MICROEJ_AUDIOPLAYER),y)
# Audio Example paths
AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio

# Audio Example flags
CXXFLAGS += $(MICROEJ_FLAGS)
CXXFLAGS += -I $(AUDIODIR)
CXXFLAGS += -I audio/
CXXFLAGS += -I core/inc/
CXXFLAGS += -I fs/inc/
CXXFLAGS += -I ui/inc/
CXXFLAGS += -I gnss/inc/
CXXFLAGS += -I osal/inc/
CXXFLAGS += -I microej_list/inc/
CXXFLAGS += -I microej_async_worker/inc/
CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LIN
CXXSRCS	 = $(wildcard audio/src/*.cxx)
endif

ASRCS =
MAINSRC	 = microjvm_main.c

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cxx=$(OBJEXT))
MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS)

BIN = $(APPDIR)$(DELIM)libmicrojvm$(LIBEXT)
INSTALL_DIR = $(BIN_DIR)
DEPLOY_DIR = $(MICROEJ_DIR)$(DELIM)microjvm$(DELIM)lib

CONFIG_MICROJVM_PROGNAME ?= microjvm$(EXEEXT)
PROGNAME = microjvm

# Build targets

all: .built
.PHONY: .built really_build clean_worker clean depend distclean

$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS) $(MAINOBJ): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

$(CXXOBJS): %$(OBJEXT): %.cxx
	$(call COMPILEXX, $<, $@)

really_build: $(OBJS)
	@touch .built


.built:
	@$(MAKE) -C TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
	@$(MAKE) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" really_build


$(DEPLOY_DIR)$(DELIM)$(PROGNAME): $(OBJS) $(MAINOBJ)
	$(Q) $(LD) $(LDELFFLAGS) $(LDLIBPATH) -o $(DEPLOY_DIR)$(DELIM)$(PROGNAME) $(ARCHCRT0OBJ) $(MAINOBJ) $(LDLIBS)
	$(Q) $(NM) -u  $(DEPLOY_DIR)$(DELIM)$(PROGNAME)

install: $(DEPLOY_DIR)$(DELIM)$(PROGNAME) 


include $(APPDIR)/Application.mk

