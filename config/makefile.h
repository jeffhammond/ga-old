# $Id: makefile.h,v 1.120 2005-01-19 18:28:22 manoj Exp $
# This is the main include file for GNU make. It is included by makefiles
# in most subdirectories of the package.
# It includes compiler flags, preprocessor and library definitions
#
# JN 03/31/2000
# 
# A note on the compiler optimization flags:
# The most aggressive flags should be set for ARMCI elsewhere.
# The code compiled with the flags set below is not floating point intensive.
# The only exception are a few lapack/blas calls used by some
# GA test programs but this should not be an issue here since
# real GA apps should use their own version of blas/lapack for best performance.
#

           FC = f77
           CC = cc
          FLD = $(FC)
          CLD = $(FLD)
           M4 = /usr/bin/m4
	CXXLD = $(CXX)
         FOPT = -O
         COPT = -O
         NOPT = -g
           AR = ar
           AS = as
       RANLIB = @echo
          CPP = /usr/lib/cpp -P
        SHELL = /bin/sh
           MV = /bin/mv
           RM = /bin/rm
      RMFLAGS = -r
      INSTALL = @echo
      ARFLAGS = rcv
    EXPLICITF = FALSE
        MKDIR = mkdir
    MAKEFLAGS = -j 1
       LINK.f = $(FLD)
       LINK.c = $(CLD)
      LINK.cc = $(CXXLD)
      LIBBLAS = -lblas
       P_FILE = YES
        CLIBS = -lm
          _FC = $(notdir $(FC))
          _CC = $(notdir $(CC))


 GLOB_DEFINES = -D$(TARGET)
     FCONVERT = $(CPP) $(CPP_FLAGS) $< > $*.f

ifdef OPTIMIZE
         FOPT = -O
         COPT = -O
endif

# to enable two underscores in fortran names, please define environment variable
# F2C_TWO_UNDERSCORES or uncomment the following line
#F2C_TWO_UNDERSCORES=1
ifndef USE_MPI
  ifeq ($(MSG_COMMS),MPI)
    F2C_TWO_UNDERSCORES=1
  endif
endif
#
#........................ SUN and Fujitsu Sparc/solaris ........................
#
ifeq ($(TARGET),SOLARIS)
          M4 = /usr/ccs/bin/m4
 ifeq ($(_CC),mpifcc)
       _CC = fcc
 endif
 ifeq ($(_FC),mpifrt)
       _FC = frt
 endif
 ifeq ($(_CC),cc)
     COPT_REN = -dalign
 endif
 ifeq ($(_FC),f77)
      FLD_REN = -xs
     FOPT_REN = -dalign
 endif
 ifeq ($(_FC),frt)
     FOPT_REN += -fw -Kfast -KV8PFMADD
     CMAIN = -Dmain=MAIN__
 endif
 ifeq ($(_CC),fcc)
      COPT_REN += -Kfast -KV8PFMADD
 endif
     ifdef LARGE_FILES
        LOC_LIBS += $(shell getconf LFS_LIBS)
     endif
 GLOB_DEFINES += -D_XOPEN_SOURCE_EXTENDED=1
endif
#
#    64-bit version
ifeq ($(TARGET),SOLARIS64)
        M4 = /usr/ccs/bin/m4
  ifeq ($(_CC),mpifcc)
       _CC = fcc
  endif
  ifeq ($(_FC),mpifrt)
       _FC = frt
  endif
  ifeq ($(_CC),fcc)
     COPT_REN = -Kfast -KV9FMADD
  else
     COPT_REN = -xarch=v9 -dalign
     ifdef USE_INTEGER4
     else
        COPT_REN += -DNO_REAL_32
     endif
  endif

  ifeq ($(_FC),frt)
#    Fujitsu SPARC systems (thanks to Herbert Fruchtl)
     FOPT_REN = -Kfast -KV9FMADD
     ifdef USE_INTEGER4
     else
       FOPT_REN += -CcdLL8 -CcdII8
     endif
     CMAIN = -Dmain=MAIN__
  else
     FOPT_REN = -xarch=v9 -dalign
     ifdef USE_INTEGER4
     else
# No 32-bit reals because of a bug in older Sun Workshop compilers
       FOPT_REN += -xtypemap=real:64,double:64,integer:64
     endif
     FLD_REN = -xs
  endif

  ifdef LARGE_FILES
        LOC_LIBS += $(shell getconf LFS_LIBS)
  endif
 GLOB_DEFINES = -DSOLARIS -DSOLARIS64
#GLOB_DEFINES += -D_XOPEN_SOURCE=1 -D_XOPEN_SOURCE_EXTENDED=1
 GLOB_DEFINES += -D_XOPEN_SOURCE_EXTENDED=1

ifdef USE_INTEGER4
else
        CDEFS = -DEXT_INT
endif
endif
#
#obsolete: SunOS 4.X
ifeq ($(TARGET),SUN)
           CC = gcc
     FOPT_REN = -Nl100 -dalign
       RANLIB = ranlib
endif
#
#................................ FUJITSU ..................................
#
#32-bit VPP5000
ifeq ($(TARGET),FUJITSU-VPP)
           FC = frt
     FOPT_REN = -Sw -KA32
     COPT_REN = -KA32
 GLOB_DEFINES = -DFUJITSU
        CMAIN = -Dmain=MAIN__
endif

#64-bit VPP5000
ifeq ($(TARGET),FUJITSU-VPP64)
           FC = frt
 GLOB_DEFINES = -DFUJITSU
        CMAIN = -Dmain=MAIN__
     FOPT_REN = -Sw
 ifdef USE_INTEGER4
 else
        CDEFS = -DEXT_INT
     FOPT_REN += -CcdLL8 -CcdII8
 endif
endif
#

#32-bit AP3000
ifeq ($(TARGET),FUJITSU-AP)
           CC = fcc
           FC = frt
     FOPT_REN = -fw
 GLOB_DEFINES = -DFUJITSU
endif
#
#................................ HITACHI ....................................
# HITACHI sr8000
#
ifeq ($(TARGET),HITACHI)
	   CC = mpicc
	   FC = mpif90 -hf77
 GLOB_DEFINES = -DHITACHI
endif
#
#................................ APPLE ....................................
# MAC running MAC X or higher
#
ifeq ($(TARGET),MACX)
           CC = gcc
           FC = g77
       RANLIB = ranlib

_FC = $(shell $(FC) -v 2>&1 | awk ' /gcc version/ { print "g77"; exit}; /xlf/ {print "xlf"; exit}  ' )

ifneq (,$(findstring mpicc,$(_CC)))
         _CC = $(shell $(CC) -v 2>&1 | awk ' /gcc version/ { print "gcc" ; exit  } ' )
endif
ifeq ($(_CC),gcc)
   ifeq ($(COPT),-O)
#    COPT_REN += -funroll-loops $(OPT_ALIGN)
#VT:unroll-loops options is causing single precision dot to fail, Is it the compiler?
     COPT_REN += $(OPT_ALIGN)
   endif
endif
#
ifeq ($(_FC),g77)
   ifeq ($(FOPT),-O)
      FOPT_REN += -O3 -funroll-loops $(OPT_ALIGN)
   endif
endif
_REQUIRE_GCCLIBPATH = $(shell $(CC) --version 2>&1 | awk '/\(GCC\) 3.3/ {print "yes";exit}; /xlc/ {print "yes";exit}')
ifeq ($(_REQUIRE_GCCLIBPATH),yes)
ifdef GCC_LIB_PATH
   CLIBS += -L$(GCC_LIB_PATH) -lgcc
   FLIBS += -L$(GCC_LIB_PATH) -lgcc
else
   CLIBS += -L/usr/lib/gcc/darwin/default -lgcc
   FLIBS += -L/usr/lib/gcc/darwin/default -lgcc
endif
endif
ifeq ($(_FC),xlf)
#    echo $_FC
     FOPT_REN +=   -qextname
GLOB_DEFINES += -DXLFMAC -DEXTNAME
endif

endif
#
#................................ LINUX ....................................
# IBM PC running Linux
#
ifeq ($(TARGET),LINUX)
           CC = gcc
           FC = g77
          CPP = gcc -E -nostdinc -undef -P
       RANLIB = ranlib
         _CPU = $(shell uname -m |\
                 awk ' /sparc/ { print "sparc" }; /i*86/ { print "x86" } ' )

ifneq (,$(findstring mpif,$(_FC)))
         _FC = $(shell $(FC) -v 2>&1 | awk ' /g77 version/ { print "g77"; exit }; /pgf/ { pgfcount++}; END {if(pgfcount)print "pgf77"} ' )
endif
ifneq (,$(findstring mpicc,$(_CC)))
         _CC = $(shell $(CC) -v 2>&1 | awk ' /gcc version/ {gcccount++}; END {if(gcccount)print "gcc"} ' )
endif
#
#              GNU compilers
ifeq ($(_CPU),x86)
     OPT_ALIGN = -malign-double
endif
ifeq ($(_CPU),786)
     OPT_ALIGN = -malign-double
endif
ifeq ($(_CC),gcc)
   ifeq ($(COPT),-O)
          COPT = -O2
     COPT_REN +=  -funroll-loops $(OPT_ALIGN)
   endif
endif
   ifneq (,$(findstring icc,$(_CC)))
       ifeq ($(COPT),-O)
           COPT_REN = -O3 -prefetch 
       endif
   endif
   ifeq ($(CC),xlc)
     COPT_REN = -q32  -qlanglvl=extended
   endif
#
#           g77
ifeq ($(_FC),g77)
   ifeq ($(FOPT),-O)
           FOPT = -O2
      FOPT_REN += -funroll-loops -fomit-frame-pointer $(OPT_ALIGN)
   endif
else
#
#             PGI fortran compiler on intel
   ifneq (,$(findstring pgf,$(_FC)))
       CMAIN = -Dmain=MAIN_
       FOPT_REN = -Mdalign -Minform,warn -Mnolist -Minfo=loop -Munixlogical
       GLOB_DEFINES += -DPGLINUX
   endif
   ifneq (,$(findstring ifc,$(_FC)))
       ifeq ($(FOPT),-O)
           FOPT_REN = -O3 -prefetch -w -cm
       endif
       GLOB_DEFINES += -DIFCLINUX
       _IFCV8= $(shell ifc -v  2>&1|egrep 8|awk ' /8\./  {print "Y"}')
       ifeq ($(_IFCV8),Y)
         GLOB_DEFINES+= -DIFCV8
       endif	
       FLD_REN += -Vaxlib
   endif
   ifneq (,$(findstring ifort,$(_FC)))
       ifeq ($(FOPT),-O)
           FOPT_REN = -O3 -prefetch -w -cm
       endif
       GLOB_DEFINES += -DIFCLINUX
       _IFCV8= $(shell ifc -v  2>&1|egrep 8|awk ' /8\./  {print "Y"}')
       ifeq ($(_IFCV8),Y)
         GLOB_DEFINES+= -DIFCV8
       endif	
       FLD_REN += -Vaxlib
       ifeq ($(LINK.c),$(FC))
          CLD_REN += -nofor_main
       endif
   endif
   ifeq ($(FC),xlf)
     FOPT_REN = -q32  -qEXTNAME
     EXPLICITF = TRUE
          CPP = /usr/bin/cpp -P -C -traditional
     GLOB_DEFINES += -DXLFLINUX -DEXTNAME
     endif
   endif

endif
#
#................................ LINUX64 ....................................
# Linux 64-bit
# Alphas running Linux
# using DEC compilers
# ia64 using Intel Compiler
# Opteron using GNU compilers with USE_INTEGER4=y
# to cross compile on x86 type: make _CPU=ia64
ifeq ($(TARGET),LINUX64)
       RANLIB = echo
GLOB_DEFINES += -DLINUX 
ifneq (,$(findstring mpif,$(_FC)))
         _FC = $(shell $(FC) -v 2>&1 | awk ' /g77 version/ { print "g77"; exit }; /efc/ { print "efc" ; exit }; /ifort/ { print "ifort" ; exit } ' )
endif
ifdef USE_INTEGER4
else
  GLOB_DEFINES += -DEXT_INT
endif
         _CPU = $(shell uname -m)
#
#-----------------------------------
# LINUX 64 CPU Specific Setup: IA64
#-----------------------------------
# IA64, only Intel fortran compiler supported 
ifeq  ($(_CPU),ia64)
           FC = efc
           CC = gcc
ifeq ($(FC),efc)
       _IFCV8= $(shell efc -V  2>&1|egrep -v Intel|egrep -v efc |egrep -i "version 8"|awk ' /8\./  {print "Y"}')
       ifeq ($(_IFCV8),Y)
         FC = ifort
         GLOB_DEFINES+= -DIFCV8
       endif	
     FOPT_REN += -cm -w90 -w95 -align 
endif
ifeq ($(FC),ifort)
       _IFCV8= $(shell efc -V  2>&1|egrep -v Intel|egrep -v efc |egrep -i "version 8"|awk ' /8\./  {print "Y"}')
       ifeq ($(_IFCV8),Y)
         GLOB_DEFINES+= -DIFCV8
       endif	
     FOPT_REN += -cm -w90 -w95 -align 
endif

ifeq ($(CC),ecc)
     COPT_REN += -fno-alias  -ftz
endif
ifeq ($(CC),icc)
     COPT_REN += -fno-alias  -ftz
endif
ifeq ($(CC),gcc) 
     COPT=-O3
     COPT_REN +=   -funroll-loops 
endif
  ifdef USE_INTEGER4
     FOPT_REN += -i4
  else
     FOPT_REN += -i8 
  endif
ifneq (,$(findstring efc,$(_FC)))
     FLD_REN += -Vaxlib
     GLOB_DEFINES += -DIFCLINUX
endif
ifneq (,$(findstring ifort,$(_FC)))
     FLD_REN += -Vaxlib
     GLOB_DEFINES += -DIFCLINUX
endif  
ifneq (,$(findstring mpif,$(_FC)))
         _FC = $(shell $(FC) -v 2>&1 | awk ' /g77 version/ { print "g77"; exit }; /efc/ { print "efc" ; exit }; /ifort/ { print "ifort" ; exit }' )
endif
          CLD = $(CC)
ifeq ($(_FC),g77)
          CLD = $(FLD)
      CLD_REN =
endif
ifeq ($(_FC),efc) 
          CLD = $(FLD)
      CLD_REN =
endif
ifeq ($(_FC),ifort) 
          CLD = $(FLD)
      CLD_REN = -nofor_main
endif
endif
#-----------------------------------
# LINUX 64 CPU Specific Setup: Alpha
#-----------------------------------
ifeq  ($(_CPU),alpha)
           CC = ccc
           FC = fort
    FOPT_REN +=-align_dcommons -fpe3 -check nooverflow 
    FOPT_REN +=-assume accuracy_sensitive -check nopower -check nounderflow
ifndef F2C_TWO_UNDERSCORES
    FOPT_REN +=-assume no2underscore
endif
ifdef USE_INTEGER4
    FLD_REN +=  -Wl,-taso
    CLD_REN+= -Wl,-taso 
endif
        CLIBS = -lfor
          CLD = $(CC)
ifeq ($(_FC),g77)
          CLD = $(FLD)
      CLD_REN =
endif
ifeq ($(_FC),fort) 
          CLD = $(FLD)
      CLD_REN =
endif
ifeq ($(_FC),efc) 
          CLD = $(FLD)
      CLD_REN =
endif
endif
#
#-------------------------------------
# LINUX 64 CPU Specific Setup: Opteron
#-------------------------------------
ifeq  ($(_CPU),x86_64)
     _FC = $(shell $(FC) -v 2>&1 | awk ' /g77 version/ { print "g77"; exit }; /gcc version/ { print "g77"; exit }; /ifc/ { print "ifort" ; exit }; /ifort/ { print "ifort" ; exit }; /efc/ { print "efc" ; exit }; /pgf90/ { pgf90count++}; /pgf77/ { pgf77count++}; END {if(pgf77count)print "pgf77" ; if(pgf90count)print "pgf90"} ')
  ifneq ($(_FC),g77)
    ifdef USE_INTEGER4
       FOPT_REN += -i4
    else
       FOPT_REN += -i8
    endif
  endif
  ifeq ($(_FC),pgf90)
#     CMAIN = -Dmain=MAIN_
     FOPT_REN += -Mdalign 
     GLOB_DEFINES += -DPGLINUX
  endif
  ifeq ($(_FC),pgf77)
     GLOB_DEFINES += -DPGLINUX
  endif
  ifeq ($(_FC),pathf90)
     FOPT_REN += -cpp
     FOPT_REN += -fno-second-underscore
     CLD_REN += -static
     COPT +=  -static
  endif
   ifeq ($(_FC),ifort)
       ifeq ($(FOPT),-O)
           FOPT_REN += -quiet  -O3 -w -cm -xW -tpp7
       endif
       GLOB_DEFINES += -DIFCLINUX -DIFCV8
   endif
   GLOB_DEFINES += -DNOUSE_MMAP
endif
#
#-------------------------------------
# LINUX 64 CPU Specific Setup: power4
#-------------------------------------
ifeq  ($(_CPU),ppc64)
  FC=xlf
  CC=/opt/cross/bin/powerpc64-linux-gcc
  COPT=-O3
  COPT_REN +=  -funroll-loops 
  ifeq ($(_CC),xlc)
     COPT_REN = -q64  -qlanglvl=extended
     GLOB_DEFINES += -DXLCLINUX
  endif
  ifeq ($(_FC),xlf)
     FOPT_REN = -q64  -qEXTNAME
     EXPLICITF = TRUE
          CPP = /usr/bin/cpp -P -C -traditional
     GLOB_DEFINES += -DXLFLINUX
     ifdef USE_INTEGER4
       FOPT_REN += -qintsize=4
     else
       FOPT_REN += -qintsize=8
     endif
  endif
endif

endif 
#
#............................. CYGNUS on Windows ..........................
#
ifeq ($(TARGET),CYGWIN)
           FC = g77
           CC = gcc
 GLOB_DEFINES = -DCYGWIN
     COPT_REN = -malign-double
       RANLIB = ranlib
endif
ifeq ($(TARGET),CYGNUS)
           FC = g77
           CC = gcc
 GLOB_DEFINES = -DLINUX -DCYGNUS
     COPT_REN = -malign-double
       RANLIB = ranlib
endif
#
ifeq ($(TARGET),INTERIX)
           FC = g77
           CC = gcc
     COPT_REN = -malign-double
endif
#
#
#................................ HP  ....................................
ifeq ($(TARGET),HPUX)
# free HP cc compiler is not up to the job: use gcc if no commercial version
#          CC = gcc
#          FC = fort77
           FC = f90

          CPP = /lib/cpp
    ifeq ($(FOPT),-O)
         FOPT = -O1
    endif
      FOPT_REN = +ppu
      COPT_REN = -Ae
        FLIBS = -lU77
 GLOB_DEFINES = -DHPUX -DEXTNAME
#   EXPLICITF = TRUE
     FCONVERT = $(CPP) $(CPP_FLAGS)  $< | sed '/^\#/D'  > $*.f
endif
#
ifeq ($(TARGET),HPUX64)
# 64-bit version
         _CPU = $(shell uname -m)
           FC = f90
    ifeq ($(FOPT),-O)
         FOPT = -O1
    endif
     FOPT_REN = +ppu 
     COPT_REN = -Ae
ifeq  ($(_CPU),ia64)
     FOPT_REN = +DD64
     COPT_REN = +DD64
else
     FOPT_REN += +DA2.0W
     COPT_REN += +DA2.0W
endif
        FLIBS = -lU77
 GLOB_DEFINES+= -DHPUX -DEXTNAME
ifdef USE_INTEGER4
#     COPT_REN +=+u1 # this is to fix alignment problems
else
     FOPT_REN += +i8
        CDEFS = -DEXT_INT
endif
endif
#
#................................ Compaq/DEC ALPHA .............................
# we use a historical name
#
ifeq ($(TARGET),DECOSF)
     FOPT_REN = -fpe2 -check nounderflow -check nopower -check nooverflow
ifdef USE_INTEGER4
     FOPT_REN += -i4 
#    COPT_REN += -misalign # alignment fix
else
     FOPT_REN += -i8 
        CDEFS = -DEXT_INT
endif
       RANLIB = ranlib
        CLIBS = -lfor -lots -lm
          CLD = $(CC)
endif
#
#................................ SGI ......................................
#
ifeq ($(TARGET),SGI)
       RANLIB = echo
     COPT_REN = -32 
     FOPT_REN = -32 
     HAS_BLAS = yes
endif

ifeq ($(TARGET),SGI_N32)
       RANLIB = echo
 GLOB_DEFINES = -DSGI -DSGI_N32
     COPT_REN = -n32 -mips4
     FOPT_REN = -n32 -mips4
     HAS_BLAS = yes
endif

ifeq ($(TARGET),SGITFP)
       RANLIB = echo
        CDEFS = -DEXT_INT
     COPT_REN = -64 -mips4 
 GLOB_DEFINES = -DSGI -DSGITFP
     FOPT_REN = -i8 -align64 -64 -mips4 
endif

ifeq ($(TARGET),SGI64)
       RANLIB = echo
 GLOB_DEFINES = -DSGI -DSGI64
     COPT_REN = -64 -mips4 
     FOPT_REN = -align64 -64 -mips4
endif
#
#................................ CRAY ..................................
# covers also J90 and SV1
#
ifeq ($(TARGET),CRAY-SV1)
     ifeq ($(FOPT), -O)
         FOPT = -O1
     endif
     COPT_REN = -htaskprivate
           FC = f90
          CPP = cpp -P -N
     FCONVERT = $(CPP) $(CPP_FLAGS)  $< | sed '/^\#/D'  > $*.f

 GLOB_DEFINES = -DCRAY_YMP -D_MULTIP_ -DCRAY_SV1
     FOPT_REN = -dp -ataskcommon
     HAS_BLAS = yes
      LIBBLAS =
    EXPLICITF = TRUE
endif
#
ifeq ($(TARGET),CRAY-YMP)
     ifeq ($(FOPT), -O)
         FOPT = -O1
     endif
     COPT_REN = -htaskprivate 
           FC = f90
          CPP = cpp -P -N
     FCONVERT = $(CPP) $(CPP_FLAGS)  $< | sed '/^\#/D'  > $*.f

 GLOB_DEFINES = -DCRAY_YMP -D_MULTIP_
     FOPT_REN = -dp -ataskcommon
     HAS_BLAS = yes
      LIBBLAS = 
    EXPLICITF = TRUE
endif
#
#
ifeq ($(TARGET),cray-sv2)
           FC = ftn
 GLOB_DEFINES =
     ifeq ($(FOPT), -O)
#        FOPT = -O vector3,msgs,negmsgs -rm
         FOPT = -O vector3
     endif
     FOPT_REN = -F -s integer64
     ifeq ($(COPT), -O)
         COPT = -O -h inline2
     endif
     CDEFS = -DEXT_INT
     LIBBLAS = 
     HAS_BLAS = yes
     ifdef USE_SSP
       FOPT_REN += -O ssp
       COPT_REN += -h ssp
     endif

#    COPT_REN = -h report=imsvf
#         CRAY = yes
endif

#
ifeq ($(TARGET),CRAY-T3D)
     ifeq ($(FOPT), -O)
         FOPT = -O1
     endif
           FC = cf77
          CPP = /mpp/lib/cpp -P -N
     FCONVERT = $(CPP) $(CPP_FLAGS)  $< | sed '/^\#/D'  > $*.f
     FOPT_REN = -Ccray-t3d -Wf-dp
    EXPLICITF = TRUE
endif
#
ifeq ($(TARGET),CRAY-T3E)
     ifeq ($(FOPT), -O)
         FOPT = -O1
     endif
           FC = f90
          CPP = cpp -P -N
     FCONVERT = $(CPP) $(CPP_FLAGS)  $< | sed '/^\#/D'  > $*.f
     FOPT_REN = -dp
 GLOB_DEFINES = -DCRAY_T3D -DCRAY_T3E
    EXPLICITF = TRUE
endif

ifeq ($(TARGET),NEC)
#
#    on SX-6 we must use c++ compiler and cc on SX-5
     CC = c++
     FC = f90
     ifeq ($(FOPT), -O)
         FOPT = -Cvopt -Wf"-pvctl nomsg noassume vwork=stack"
     endif
     ifeq ($(COPT), -O)
         COPT = -V -Cvsafe -O nomsg -pvctl,nomsg -Xa
     endif
     CLD = $(FC) -size_t64
     LINK.c = $(CLD)
     FLD_REN  = -size_t64
#     COPT_REN = -hsize_t64
     FOPT_REN = -ew -size_t64
     CDEFS    = -size_t64 -DEXT_INT
#    CLIBS    = -li90sxe
     CLIBS    = -f90libew
     LIBBLAS = -lblas
     HAS_BLAS = yes
endif
#
#.............................. IBM .........................................
# LAPI is the primary target for SP
#
ifeq ($(TARGET),LAPI)
         IBM_ = 1
         FLD  = mpcc_r -lxlf -lxlf90 -lm
           CC = mpcc_r
GLOB_DEFINES += -DSP
endif

ifeq ($(TARGET),LAPI64)
         IBM_ = 1
         FLD  = mpcc_r -lxlf -lxlf90 -lm
           CC = mpcc_r
     FOPT_REN = -q64 
     COPT_REN = -q64
ifdef USE_INTEGER4
   FOPT_REN += -qintsize=4
else
   FOPT_REN += -qintsize=8
        CDEFS = -DEXT_INT
endif
      ARFLAGS = -rcv 
      AR = ar -X 64
GLOB_DEFINES += -DSP -DLAPI
endif

#....................
ifeq ($(TARGET),SP1)
#
         IBM_ = 1
         FLD  = mpxlf
           CC = mpcc
endif
#....................
ifeq ($(TARGET),SP)
#
         IBM_ = 1
         FLD  = mpxlf
           CC = mpcc

# need to strip symbol table to alleviate a bug in AIX 4.1 ld
define AIX4_RANLIB
  ranlib $@
  strip
endef
       RANLIB = $(AIX4_RANLIB)
endif

ifeq ($(TARGET),IBM)
# IBM RS/6000 under AIX  
#
         IBM_ = 1
GLOB_DEFINES =
endif

ifeq ($(TARGET),IBM64)
# 64-bit port, 8-byte fortran integers
         IBM_ = 1
     FOPT_REN = -q64 
     COPT_REN = -q64
ifdef USE_INTEGER4
   FOPT_REN += -qintsize=4
else
   FOPT_REN += -qintsize=8
        CDEFS = -DEXT_INT
endif
      ARFLAGS = -rcv -X 64
endif


ifdef IBM_
           FC = xlf
     FOPT_REN += -qEXTNAME -qarch=auto
GLOB_DEFINES += -DIBM -DAIX
       CDEFS += -DEXTNAME
    EXPLICITF = TRUE
# we compile blas to avoid headache with missing underscores in the IBM library
# testsolve.x uses several blas routines
#     HAS_BLAS = yes
endif
 
ifdef GA_USE_VAMPIR
   GLOB_DEFINES += -DGA_USE_VAMPIR
   ifdef VT_DEBUG
      GLOB_DEFINES += -DVT_DEBUG
   endif
endif
#
#.............................. final flags ....................................
#

#get rid of 2nd underscore under g77
ifeq ($(_FC),g77)
ifndef F2C_TWO_UNDERSCORES
     FOPT_REN += -fno-second-underscore
endif
     ifndef OLD_G77
        FOPT_REN += -Wno-globals
     endif
endif

#add 2nd underscore under linux/cygwin to match g77 names
ifdef F2C_TWO_UNDERSCORES
     CDEFS += -DF2C2_
endif

       DEFINES = $(GLOB_DEFINES) $(LIB_DEFINES)

ifdef GA_C_CORE
  DEFINES += -DGA_C_CORE
endif

# If user specifies a BLAS library instead of the NATIVE(/vendor) blas
ifeq ($(NATIVE_BLAS), no)
     HAS_BLAS = 
     LIBBLAS = $(BLAS_LIB)
endif

ifeq ($(HAS_BLAS),yes)
  DEFINES += -DHAS_BLAS
endif

ifeq ($(MSG_COMMS),MPI)
  INCLUDES += $(MP_INCLUDES) 
ifndef __MPIPP
  DEFINES += -DMPI
endif
ifdef __MPIPP
  DEFINES += -DMPIPP
endif
endif



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

       FFLAGS = $(FOPT) $(FOPT_REN) 
       CFLAGS = $(INCLUDES) $(DEFINES) $(COPT) $(COPT_REN) $(CDEFS) $(LIB_CDEFS)
       CFLAGS := $(strip $(CFLAGS))
       FFLAGS := $(strip $(FFLAGS))
       FLDOPT =  $(FLD_REN)
       CLDOPT =  $(CLD_REN)


ifeq ($(LINK.f),$(FC))
       FLDOPT += $(FOPT_REN)
else
       FLDOPT += $(COPT_REN)
endif

ifeq ($(LINK.cc),$(FC))
       CXXLDOPT = $(CLD_REN)
       CXXLDOPT += $(FOPT_REN)
else
       CXXLDOPT = $(CLD_REN)
       CXXLDOPT += $(COPT_REN)
endif

ifeq ($(LINK.c),$(FC))
       CLDOPT += $(FOPT_REN)
else
       CLDOPT += $(COPT_REN)
endif

CXXFLAGS = $(CFLAGS)    


#
# Define known suffixes mostly so that .p files dont cause pc to be invoked
#
.SUFFIXES:	
.SUFFIXES:	.o .s .F .f .c .m4 .cc

ifeq ($(EXPLICITF), TRUE)
#
# Needed on machines where FCC does not preprocess .F files
# with CPP to get .f files
#
.SUFFIXES:	
.SUFFIXES:	.o .s .F .f .c .m4 .cc

.m4.o:
	$(M4) $*.m4 > $*.F
	$(MAKE) $*.f
	$(FC) $(FOPT_REN) -c $*.f
	$(RM) -f $*.F $*.f

.F.o:	
	@echo Converting $*.F '->' $*.f
	@$(FCONVERT)
	$(FC) -c $(FFLAGS) $*.f
	@$(RM) $*.f

.F.f:
	@echo Converting $*.F '->' $*.f
	$(FCONVERT)
else

.SUFFIXES:      .m4

.m4.o:
	$(M4) $*.m4 > $*.F
	$(FC) $(CPP_FLAGS) $(FOPT_REN) -c $*.F -o $*.o
	$(RM) $*.F

endif

# 
# More explicit rules to avoid infinite recursion, to get dependencies, and
# for efficiency.  CRAY does not like -o with -c.

%.o:	%.F
ifeq ($(EXPLICITF),TRUE)
	@echo Converting $< '->' $*.f
	$(FCONVERT)
ifeq (CRAY,$(findstring CRAY,$(TARGET)))
	$(FC) -c $(FFLAGS) $*.f
else
	$(FC) -c $(FFLAGS) -o $@ $*.f
endif
	@/bin/rm -f $*.f
else
	$(FC) -c $(FFLAGS) $(CPP_FLAGS) $<
endif

ifeq (CRAY,$(findstring CRAY,$(TARGET)))
%.o:	%.f
	$(FC) -c $(FFLAGS) $*.f
endif

#
#.................. libraries for test programs ...............................
# Almost every library in the package contains its test programs.
# LIBS contains definitions of libraries used by these programs.
# LOC_LIBS defines extra libraries required by test programs for each library 
# This is rather complicated because of all different configurations and 
# options supported:
# We create list of libs needed by test programs in each of
# the subdirectories by concatenating library definitions for
# linear algebra, ARMCI, message-passing library, and any lower level libs
#
# core libs
LIBS = -L$(LIB_DISTRIB)/$(TARGET) -lglobal -lma 
#
#linear algebra
ifdef USE_SCALAPACK
  LIBS += $(SCALAPACK)
endif
LIBS += -llinalg $(LOC_LIBS)

ifeq ($(HAS_BLAS),yes)
  LIBS += $(LIBBLAS)
endif
ifeq ($(NATIVE_BLAS),no)
  LIBS += $(BLAS_LIB)
endif

#
#communication libs
LIBS += -larmci

ifndef LIBMPI
   LIBMPI = -lmpi
endif
ifdef MPI_LIB
   LIBS += -L$(MPI_LIB)
endif

ifdef USE_MPI
  ifdef GA_USE_VAMPIR
      ifdef VT_LIB
         ifdef LIBVT
            LIBS += -ltcgmsg-mpi -L$(VT_LIB) $(LIBVT) $(LIBMPI)
         else
            LIBS += -ltcgmsg-mpi -L$(VT_LIB) -lVT $(LIBMPI)
         endif
#     else
#	Setenv VT_PATH to -L<directory where libVT.a lives>
      endif
      ifdef VT_INCLUDE
         INCLUDES += -I$(VT_INCLUDE) 
      endif
  else
      LIBS += -ltcgmsg-mpi $(LIBMPI)
  endif
else
  ifeq ($(MSG_COMMS),MPI)
    LIBS += $(MP_LIBS)
  else
    ifdef GA_USE_VAMPIR
      ifdef VT_LIB
         ifdef LIBVT
            LIBS += -ltcgmsg -L$(VT_LIB) $(LIBVT) $(LIBMPI)
         else
            LIBS += -ltcgmsg -L$(VT_LIB) -lVT  $(LIBMPI)
         endif
#     else
#        Setenv VT_PATH to -L<directory where libVT.a lives>
      endif
      ifdef VT_INCLUDE
         INCLUDES += -I$(VT_INCLUDE) 
      endif
    else
      LIBS += -ltcgmsg 
    endif
  endif
endif

# lower level libs used by communication libraries
ifdef COMM_LIBS
  LIBS += $(COMM_LIBS)
endif

LIBS += -lm
#........................... End ..............................................
