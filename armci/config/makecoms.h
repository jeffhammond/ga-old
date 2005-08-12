#provides definitions for protocols used by communication libraries
# COMM_DEFINES - identifies communication network protocol
# COMM_LIBS    - list of libs that need to be linked with 
# COMM_INCLUDES- include path to access communication protocol API
#
ifeq ($(ARMCI_NETWORK),GM)
  COMM_DEFINES = -DGM
  ifdef GM_INCLUDE
    COMM_INCLUDES = -I$(GM_INCLUDE)
  endif
  ifdef GM_LIB
    COMM_LIBS = -L$(GM_LIB)
  endif
  GM_LIB_NAME = -lgm
  COMM_LIBS += $(GM_LIB_NAME) -lpthread
endif

ifeq ($(ARMCI_NETWORK),PORTALS)
  COMM_DEFINES = -DPORTALS -DP3_NAL=\<p3nal\_utcp\.h\>
  ifdef PORTALS_PATH
    COMM_INCLUDES = -I$(PORTALS_PATH)/include
    COMM_LIBS = -L$(PORTALS_PATH)/nal/utcp -L$(PORTALS_PATH)/user -L$(PORTALS_PATH)/kern
  else
    COMM_INCLUDES = -I/usr/local/include
    COMM_LIBS = -L/usr/local/lib
  endif
  PORTALS_LIB_NAME = -l$(PORTALS_NAL)lib -lp3api -lp3lib -lp3rt
  COMM_LIBS += $(PORTALS_LIB_NAME) -lpthread
endif

ifeq ($(ARMCI_NETWORK),VIA)
  COMM_DEFINES = -DVIA
  ifdef VIA_INCLUDE
    COMM_INCLUDES = -I$(VIA_INCLUDE)
  endif
  ifdef VIA_LIB
    COMM_LIBS = -L$(VIA_LIB)
  endif
  VIA_LIB_NAME = -lvipl
  COMM_LIBS += $(VIA_LIB_NAME)
endif

ifeq ($(ARMCI_NETWORK),MELLANOX)
  COMM_DEFINES = -DMELLANOX
  ifdef IB_INCLUDE
    COMM_INCLUDES = -I$(IB_INCLUDE)
  endif
  ifdef IB_LIB
    COMM_LIBS = -L$(IB_LIB)
  endif
  IB_LIB_NAME = -lvapi -lmosal -lmtl_common -lmpga
  COMM_LIBS += $(IB_LIB_NAME)
endif

ifeq ($(ARMCI_NETWORK),ELAN3)
  ifdef ELAN3_INCLUDE
        QUADRICS_INCLUDE = $(ELAN3_INCLUDE)
  endif
  ifdef ELAN3_LIB
        QUADRICS_LIB = $(ELAN3_LIB)
  endif
  ARMCI_NETWORK = QUADRICS
endif

ifeq ($(ARMCI_NETWORK),QUADRICS)
  COMM_DEFINES = -DQUADRICS
  ifdef QUADRICS_INCLUDE
    COMM_INCLUDES = -I$(QUADRICS_INCLUDE)
  else
    ifeq ($(TARGET),DECOSF)
       COMM_INCLUDES = -I/usr/opt/rms/include
    endif
  endif
  ifdef QUADRICS_LIB
    COMM_LIBS = -L$(QUADRICS_LIB) -Wl,-rpath-link -Wl,$(QUADRICS_LIB)
  else
    ifeq ($(TARGET),DECOSF)
      COMM_LIBS = -L/usr/opt/rms/lib
    endif
  endif
  QUADRICS_LIB_NAME = -lshmem -lelan -lpthread
  COMM_LIBS += $(QUADRICS_LIB_NAME)
endif

ifeq ($(ARMCI_NETWORK),ELAN4)
  ifdef ELAN4_INCLUDE
    COMM_INCLUDES = -I$(ELAN4_INCLUDE)
  else
    ifeq ($(TARGET),DECOSF)
       COMM_INCLUDES = -I/usr/opt/rms/include
    else
       COMM_INCLUDES = -I/usr/lib/qsnet/elan4/include
    endif
  endif
  ifdef ELAN4_LIB
    COMM_LIBS = -L$(ELAN4_LIB) -Wl,-rpath-link -Wl,$(ELAN4_LIB)
  else
    ifeq ($(TARGET),DECOSF)
      COMM_LIBS = -L/usr/opt/rms/lib
    else
      COMM_LIBS = -L/usr/lib/qsnet/elan4/lib
    endif
  endif
  COMM_DEFINES += -DDOELAN4  -DQUADRICS
  ELAN4_LIB_NAME = -lelan4 -lelan -lpthread
  COMM_LIBS += $(ELAN4_LIB_NAME)
endif
    
ifeq ($(TARGET),LINUX64)
# _ALTIX= $(shell /bin/rpm -q -i sgi-mpt  2>&1|egrep Reloc|awk ' /Rel/  {print "Y"}')
     _SGIALTIX= $(shell if [ -r /proc/sgi_sn/system_serial_number ]; then /bin/echo Y; fi)
ifeq ($(_SGIALTIX),Y)
  COMM_LIBS += -lsma
endif
endif

ifeq ($(TARGET),LAPI)
ifdef LAPI2
  COMM_DEFINES += -DLAPI2
  COMM_INCLUDES = -I/u2/d3h325/lapi_vector_beta
  COMM_LIBS += /u2/d3h325/lapi_vector_beta/liblapi_r_dbg.a
endif
endif
ifeq ($(TARGET),LAPI64)
   COMM_LIBS += $(LAPI64LIBS)
endif

ifeq ($(TARGET),SOLARIS)
#  need gethostbyname from -lucb under earlier versions of Solaris
   COMM_LIBS += $(shell uname -r |\
                awk -F. '{ if ( $$1 == 5 && $$2 < 6 )\
                print "/usr/ucblib/libucb.a" }')
   COMM_LIBS +=  -lsocket -lrpcsvc -lnsl
endif
ifeq ($(TARGET),SOLARIS64)
   COMM_LIBS +=  -lsocket -lrpcsvc -lnsl
endif

ifeq ($(TARGET),FUJITSU-VPP)
   COMM_LIBS = -lmp -lgen -lpx -lelf -Wl,-J,-P
endif

ifeq ($(TARGET),FUJITSU-VPP64)
   COMM_LIBS = -lmp -lgen -lpx -lelf -Wl,-J,-P
endif
   
ifeq ($(TARGET),FUJITSU-AP)
   COMM_LIBS = -L/opt/FSUNaprun/lib -lmpl -lelf -lgen
endif

ifeq ($(TARGET),CRAY-YMP)
   COMM_LIBS = $(LIBCM)
endif

