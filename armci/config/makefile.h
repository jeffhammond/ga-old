           FC = f77
           CC = cc
           AR = ar
           AS = as
       RANLIB = echo
          CPP = /usr/lib/cpp
        SHELL = /bin/sh
           MV = /bin/mv
           RM = /bin/rm
      RMFLAGS = -r
      INSTALL = @echo
    MAKEFLAGS = -j 1                                                            
      ARFLAGS = rcv
        MKDIR = mkdir
       LINK.f = $(FLD)
       LINK.c = $(CLD)
 GLOB_DEFINES = -D$(TARGET)
          CLD = $(CC)
          _FC = $(notdir $(FC))
          _CC = $(notdir $(CC))
      COPT_NO = -g
      FOPT_NO = -g

#-------------------------- Cygwin/Cygnus: GNU on Windows ------------
ifeq ($(TARGET),CYGNUS) 
           FC = g77
           CC = gcc
     COPT_REN = -malign-double
 GLOB_DEFINES+= -DLINUX
endif

ifeq ($(TARGET),CYGWIN)
           FC = g77
           CC = gcc
 GLOB_DEFINES+= -DSHMEM -DMMAP -DSYSV
     COPT_REN = -malign-double
     EXTRA_OBJ = winshmem.o signaltrap.o shmalloc.o
endif

#-------------------------- Mac X ------------
ifeq ($(TARGET),MACX)
       RANLIB = ranlib
           FC = g77
           CC = gcc
 GLOB_DEFINES+= -DSHMEM -DMMAP -DDATA_SERVER
     EXTRA_OBJ = winshmem.o signaltrap.o shmalloc.o dataserv.o spawn.o \
                 dataserv.o sockets.o request.o ds-shared.o buffers.o async.o
endif

#-------------------------- INTERIX 2.2.5 on Windows ------------
ifeq ($(TARGET),INTERIX) 
           FC = g77
           CC = gcc
     COPT_REN = -malign-double
endif
 
#------------------------------- Linux -------------------------------
ifeq ($(TARGET),LINUX)
       RANLIB = ranlib
           FC = g77
           CC = gcc
         _CPU = $(shell uname -m |\
                 awk ' /sparc/ { print "sparc" };/ppc/{ print "ppc"};\
                     /i686/{ print "686" }; /i*86&&^i686/ { print "x86" } ' )

ifneq (,$(findstring mpif,$(_FC)))
         _FC = $(shell $(FC) -v 2>&1 | awk ' /g77 version/ { print "g77"; exit }; /pgf/ { apgfcount++}; END {if(apgfcount)print "pgf77"} ' )
endif
ifneq (,$(findstring mpicc,$(_CC)))
         _CC = $(shell $(CC) -v 2>&1 | awk ' /gcc version/ {agcccount++}; END {if(agcccount)print "gcc"} ' )
endif
#
#              GNU compilers 
ifeq ($(_CPU),ppc)
        CDEFS += -DPPC
endif
ifeq ($(_CPU),x86)
     OPT_ALIGN = -malign-double
endif
ifeq ($(_CPU),686)
     OPT_ALIGN = -malign-double -march=pentiumpro
        CDEFS += -DCOPY686
    EXTRA_OBJ += x86copy.o
endif
ifeq ($(_CPU),786)
     OPT_ALIGN = -malign-double -march=pentiumpro
#        CDEFS += -DCOPY686
    EXTRA_OBJ += x86copy.o
endif
ifeq ($(_CC),gcc)
   ifeq ($(COPT),-O)
          COPT = -O2 -finline-functions -funroll-loops
     COPT_REN += -Wall $(OPT_ALIGN)
   endif
else
   EXTRA_OBJ += tas.o
endif
#
#           g77
ifeq ($(_FC),g77)
   ifeq ($(FOPT),-O)
         FOPT = -O3
    FOPT_REN += -funroll-loops -fomit-frame-pointer $(OPT_ALIGN)
   endif
else
#
#             PGI fortran compiler on intel
   ifneq (,$(findstring pgf,$(_FC)))
       FOPT_REN = -Mvect  -Munroll -Mdalign -Minform,warn -Mnolist -Minfo=loop -Munixlogical
ifeq ($(_CPU),686)
       FOPT_REN +=  -tp p6
endif
   endif
   ifneq (,$(findstring ifc,$(_FC)))
       FOPT=-O4 -prefetch -unroll -ip
ifeq ($(_CPU),k7)
       FOPT_REN = -xM 
endif
ifeq ($(_CPU),686)
       FOPT_REN = -xK -tpp6
endif
ifeq ($(_CPU),786)
       FOPT_REN = -xW -tpp7
endif
   endif
endif


endif
#-----------------Linux 64-bit on DEC/Compaq Alpha with DEC compilers --
#-----------------Linux 64-bit on Itanium with Intel compilers --
ifeq ($(TARGET),LINUX64)
   GLOB_DEFINES += -DLINUX
         _CPU = $(shell uname -m)

ifneq (,$(findstring mpicc,$(_CC)))
         _CC = $(shell $(CC) -v 2>&1 | awk ' /gcc version/ { print "gcc" ; exit  } ' )
endif

ifeq  ($(_CPU),ia64)
     FC=efc
     CC=gcc
  ifeq ($(_FC),sgif90)
     FOPT_REN = -macro_expand 
  endif
  ifeq ($(_FC),efc)
   GLOB_DEFINES += -DIFCLINUX
   FOPT_REN= -w -cm -w90 #-align 
#
#  for IA64 only. gcc 3.x cannot find the symbols modsi3 and divsi3 in IA64.
#  lib1funs-ia64 includes these symbols.
   EXTRA_OBJ += funcs-ia64.o

   ifeq ($(FOPT),-O)
     FOPT =  -O3 -hlo -ftz -pad
   endif
  endif
  ifeq ($(_CC),gcc)
      COPT_NO = -g -O0
#     COPT= -O3
  endif
  ifeq ($(_CC),ecc)
     COPT_REN= -w1 #-fno-alias    
  endif
  GLOB_DEFINES += -DNEED_MEM_SYNC
endif

ifeq  ($(_CPU),alpha)
     FC = fort
     CC = ccc
     FOPT_REN = -assume no2underscore -fpe3 -check nooverflow
     FOPT_REN+= -assume accuracy_sensitive -check nopower -check nounderflow
endif
     EXTRA_OBJ += tas.o

   
endif
#----------------------------- Fujitsu ------------------------------
ifeq ($(TARGET),FUJITSU-VPP)
           FC = frt
     FOPT_REN = -Sw -KA32
     COPT_REN = -x100 -KA32
 GLOB_DEFINES = -DFUJITSU -DVPP_STRIDED_READ -DVPP_STRIDED_WRITE
#   EXTRA_LIBS = /usr/local/lib/libmp.a -L/opt/tools/lib/ -lgen  -lpx -lelf -Wl,-J,-P
endif

ifeq ($(TARGET),FUJITSU-VPP64)
           FC = frt
     FOPT_REN = -Sw
     COPT_REN = -x100
 GLOB_DEFINES = -DFUJITSU -DFUJITSU64 
#disable if broken
 GLOB_DEFINES += -DVPP_STRIDED_READ -DVPP_STRIDED_WRITE
endif

#AP3000 running Solaris on Sparc
ifeq ($(TARGET),FUJITSU-AP)
           FC = frt
           CC = fcc
#     FOPT_REN = 
 GLOB_DEFINES = -DFUJITSU
endif

#---------------------------- Hitachi SR8000 ------------------------------
ifeq ($(TARGET),HITACHI)
           FC = f90 -hf77
           CC = mpicc
      ifeq ($(FOPT),-O)
         FOPT = -opt=ss -nopar
      endif
   EXTRA_OBJ += sr8k.o
endif

#---------------------------- Sun or Fujitsu Sparc/Solaris ----------------
ifeq ($(TARGET),SOLARIS)
  ifeq ($(_CC),mpifcc)
       _CC = fcc
  endif
  ifeq ($(_FC),mpifrt)
       _FC = frt
  endif
  ifeq ($(_FC),f77)
      FOPT_REN = -dalign
  endif
  ifeq ($(_CC),cc)
      COPT_REN = -dalign
  endif
  ifeq ($(_FC),frt)
      FOPT_REN += -fw -Kfast -KV8PFMADD
  endif
  ifeq ($(_CC),fcc)
      COPT_REN += -Kfast -KV8PFMADD
  endif
endif
#
# 64-bit
ifeq ($(TARGET),SOLARIS64)
#
  ifeq ($(_CC),mpifcc)
       _CC = fcc
  endif
  ifeq ($(_FC),mpifrt)
       _FC = frt
  endif
  ifeq ($(_FC),frt)
     FOPT_REN = -fw -Kfast -KV9FMADD
  else
     FOPT_REN = -dalign -xarch=v9
  endif
  ifeq ($(_CC),fcc)
     COPT_REN = -Kfast -KV9FMADD
  else
     COPT_REN = -dalign -xarch=v9
  endif
#
 GLOB_DEFINES += -DSOLARIS
endif
#
#
#obsolete SunOS 4.X
ifeq ($(TARGET),SUN)
           CC = gcc
     FOPT_REN = -Nl100
       RANLIB = ranlib
endif

#----------------------------- HP/Convex ------------------------------
ifeq ($(TARGET),HPUX)
           FC = fort77
           AS = cc -c
    ifeq ($(FOPT),-O)
         FOPT = -O3
         FOPT += $(shell uname -m |\
		 awk -F/ '{ if ( $$2 > 799 ) print "+Odataprefetch" }')
    endif
     FOPT_REN = +ppu
     COPT_REN = -Ae
       CDEFS += -DEXTNAME
#   EXTRA_OBJ = tas-parisc.o
endif
#
ifeq ($(TARGET),HPUX64)
         _CPU = $(shell uname -m)
           FC = f90
           AS = cc -c
     FOPT_REN = +ppu
     COPT_REN = -Ae 
ifeq  ($(_CPU),ia64)
     FOPT_REN = +DD64
     COPT_REN = +DD64
else
     FOPT_REN += +DA2.0W 
     COPT_REN += +DA2.0W 
    ifeq ($(FOPT),-O)
         FOPT = -O3 +Odataprefetch +Ofastaccess
    endif
endif
       CDEFS += -DEXTNAME
GLOB_DEFINES += -DHPUX
endif
#
#
ifeq ($(TARGET),CONVEX-SPP)
           FC = fc
          CPP = /lib/cpp
    ifeq ($(FOPT),-O)
         FOPT = -O1
    endif
    ifeq ($(FOPT),-g)
         FOPT = -no
    endif
    ifeq ($(COPT),-g)
         COPT = -no
    endif
    COPT_REN  = -or none
     FOPT_REN = -ppu -or none
 GLOB_DEFINES = -DCONVEX
endif

#----------------------------- SGI ---------------------------------
ifeq ($(TARGET),SGI)
#    COPT_REN = -32
#    FOPT_REN = -32
    SGI = yes
endif

ifeq ($(TARGET),SGI_N32)
    COPT_REN = -n32
    FOPT_REN = -n32
    SGI = yes
endif

ifeq ($(TARGET),SGI64)
    COPT_REN = -64
    FOPT_REN = -align64 -64
    SGI = yes
endif

ifeq ($(TARGET),SGITFP)
    COPT_REN = -64
    FOPT_REN = -align64 -64
GLOB_DEFINES += -DSGI64
    SGI = yes
endif

ifdef SGI
GLOB_DEFINES += -DSGI
# optimization flags for R10000 (IP28)
  FOPT_R10K = -TENV:X=1 -WK,-so=1,-o=1,-r=3,-dr=AKC
# optimization flags for R8000 (IP21)
  FOPT_R8K = -TENV:X=3 -WK,-so=1,-o=1,-r=3,-dr=AKC
    ifeq ($(FOPT),-O)
         FOPT = -O3
    endif

#CPU specific compiler flags
ifneq ($(TARGET_CPU),R4000)
    COPT_REN += -mips4
    FOPT_REN += -mips4
else
     COPT_REN = -32
     FOPT_REN = -32
endif

ifdef TARGET_CPU

ifeq ($(TARGET_CPU),R10000)
 FOPT_REN += $(FOPT_R10K)
endif
ifeq ($(TARGET_CPU),R8000)
 FOPT_REN += $(FOPT_R8K)
endif

else
    FOPT_REN += $(FOPT_R10K)
endif

endif

#----------------------------- DEC/Compaq ---------------------------------
ifeq ($(TARGET),DECOSF)
          CLD = cc
     FOPT_REN = -fpe2 -check nounderflow -check nopower -check nooverflow
     COPT_REN += -D_POSIX_PII_SOCKET
endif
#------------------------------- Crays ------------------------------------

# YMP, J90, ... PVP
#
ifeq ($(TARGET),CRAY-YMP)
     COPT_REN = -htaskprivate $(LIBCM)
           FC = f90
 GLOB_DEFINES = -DCRAY_YMP
     FOPT_REN = -dp -ataskcommon $(LIBCM)
         CRAY = yes
endif

ifeq ($(TARGET),CRAY-SV1)
     COPT_REN = -htaskprivate $(LIBCM)
           FC = f90
 GLOB_DEFINES = -DCRAY_YMP -DCRAY_SV1
     FOPT_REN = -dp -ataskcommon $(LIBCM)
         CRAY = yes
endif


ifeq ($(TARGET),CRAY-T3D)
           FC = cf77
 GLOB_DEFINES = -DCRAY_T3D
         CRAY = yes
endif


ifeq ($(TARGET),CRAY-T3E)
           FC = f90
     FOPT_REN = -dp
 GLOB_DEFINES = -DCRAY_T3E
         CRAY = yes
endif

ifdef CRAY
     ifeq ($(FOPT), -O)
         FOPT = -O2
     endif
     ifeq ($(COPT), -O)
         COPT = -O1 -hinline3
     endif
endif
#................................. NEC SX-4 ..................................
ifeq ($(TARGET),NEC)
#
     FC = f90
     ifeq ($(FOPT), -O)
         FOPT = -Cvopt -Wf"-pvctl nomsg"
     endif
     ifeq ($(COPT), -O)
         COPT = -O nomsg -hnovector,nomulti -pvctl,nomsg
     endif
#    COPT_REN = -hsize_t64
     EXTRA_LIBS += -li90sx
endif

#................................. IBM SP and workstations ...................

# IBM SP with LAPI: 32- and 64-bit versions
ifeq ($(TARGET),LAPI)
         IBM_ = 1
         LAPI_= 1
endif
ifeq ($(TARGET),LAPI64)
       IBM64_ = 1
         LAPI_= 1
GLOB_DEFINES += -DLAPI -DIBM64
endif
#
# IBM RS/6000 under AIX
ifeq ($(TARGET),IBM)
        IBM_  = 1
endif
ifeq ($(TARGET),IBM64)
      IBM64_  = 1
endif
#
ifdef LAPI_
          CC  = mpcc_r
      LINK.f  = mpcc_r -lc_r -lxlf -lxlf90 -lm
    EXTRA_OBJ = lapi.o request.o buffers.o
GLOB_DEFINES += -DSP
endif
#
ifdef IBM64_
        IBM_  = 1
     FOPT_REN = -q64
     COPT_REN = -q64
      ARFLAGS = -rcv -X 64
endif
#
ifdef IBM_
     ifeq ($(FOPT), -O)
         FOPT = -O4 -qarch=auto -qstrict
     else
#        without this flag xlf_r creates nonreentrant code
         FOPT_REN += -qnosave
     endif
     ifeq ($(COPT), -O)
         COPT = -O3 -qinline=100 -qstrict -qarch=auto -qtune=auto
     endif
       CDEFS += -DEXTNAME
           FC = xlf
GLOB_DEFINES += -DAIX
         _CPU = $(shell lsattr -El `lsdev -C -c processor -F name | head -1` | awk ' /POWER4/ { print "PWR4" };')
endif
#
ifeq ($(_CPU),PWR4)
GLOB_DEFINES += -DNEED_MEM_SYNC
endif
#
#...................... common definitions .......................
#

#alternative malloc library
ifdef ALT_MALLOC
EXTRA_LIBS += $(ALT_MALLOC)
endif

#get rid of 2nd underscore under g77
ifeq ($(_FC),g77)
     FOPT_REN += -fno-second-underscore
endif


       DEFINES = $(GLOB_DEFINES) $(LIB_DEFINES)

#Fujitsu fortran compiler requires -Wp prefix for cpp symbols
ifeq ($(TARGET),FUJITSU-VPP)
       comma:= ,
       empty:=
       space:= $(empty) $(empty)
       FDEFINES_0 = $(strip  $(DEFINES))
       FDEFINES = -Wp,$(subst $(space),$(comma),$(FDEFINES_0))
else
       FDEFINES = $(DEFINES)
endif

       INCLUDES += $(LIB_INCLUDES)
       CPP_FLAGS += $(INCLUDES) $(FDEFINES)

ifdef FRAGILE
       FOPT= $(FOPT_NO)
       COPT= $(COPT_NO)
endif

       FFLAGS = $(FOPT) $(FOPT_REN)
       CFLAGS = $(COPT) $(COPT_REN) $(INCLUDES) $(DEFINES) $(CDEFS) $(LIB_CDEFS)
       CFLAGS := $(strip $(CFLAGS))
       FFLAGS := $(strip $(FFLAGS))

ifeq (CRAY,$(findstring CRAY,$(TARGET)))
%.o:    %.f
	$(FC) -c $(FFLAGS) $*.f
endif

%.o:    %.gcc
	gcc -x c -c $*.gcc
