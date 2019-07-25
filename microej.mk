############################################################################
#  Copyright 2019 Sony Corp . All rights reserved.
#  This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
#  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
############################################################################

############################################################################
# MicroEJ Platform Path
############################################################################
MICROEJ_DIR = $(SDKDIR)$(DELIM)..$(DELIM)MicroEJ

MICROEJ_JAVA_OBJ_PATH = $(MICROEJ_DIR)$(DELIM)microej$(DELIM)lib

MICROEJ_JAVA_OBJ_NAME = microejapp$(OBJEXT)

MICROEJ_FLAGS += -I $(MICROEJ_DIR)$(DELIM)microej$(DELIM)inc
CFLAGS += -Wno-pointer-to-int-cast
CFLAGS += -Wno-pointer-sign
CFLAGS += -Wno-strict-prototypes
CFLAGS += -Wno-incompatible-pointer-types

############################################################################
# MicroEJ Platform Path
############################################################################
