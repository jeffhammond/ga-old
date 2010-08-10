# Process this file with autoconf to produce a configure script.

###############################################################################
# Init autoconf
###############################################################################
AC_PREREQ([2.64])
AC_INIT([Global Arrays (GA)],[5.0],[hpctools@pnl.gov], [ga],
        [http://www.emsl.pnl.gov/docs/global/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_LIBOBJ_DIR([compat])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([LinAlg/lapack+blas/dummy.c])

###############################################################################
# Init automake
###############################################################################
AM_INIT_AUTOMAKE([color-tests foreign parallel-tests silent-rules subdir-objects])

# Don't emit "rebuild rules" for configure, Makefile.ins, etc.
AM_MAINTAINER_MODE

###############################################################################
# Misc. information and package setup.
###############################################################################
GA_TARGET

GA_WITH_HELP

GA_TOP_BUILDDIR="`pwd`"
cd "$srcdir"
GA_TOP_SRCDIR="`pwd`"
cd "$GA_TOP_BUILDDIR"
AS_IF([test "$GA_TOP_BUILDDIR" != "$GA_TOP_SRCDIR"],
      [AC_MSG_NOTICE([Detected VPATH build])])

# Determine messaging library up front because if MPI is desired we use the
# MPI compiler wrappers instead of the standard compilers.
GA_MSG_COMMS

# Whether to disable/enable Fortran 77 or C++ bindings.
# Note that compiling the Fortran 77 source is enabled by default.
GA_F77_DISABLE
GA_CXX_ENABLE

GA_DISABLE_MPI_TESTS
GA_ENABLE_CHECKPOINT
GA_ENABLE_OPT
GA_ENABLE_PEIGS
GA_ENABLE_TRACE
GA_THREAD_SAFE
GA_WITH_VAMPIR

# Unless we rename all preprocessor symbols of MPI and MPIPP
# in the source code, this hack is necessary.
AH_BOTTOM([/* DO NOT MODIFY THE FOLLOWING PREPROCESSOR DIRECTIVES */
#ifdef MSG_COMMS_MPI
#   ifdef __cplusplus
#       define MPIPP
#   else
#       define MPI
#   endif /* __cplusplus */
#endif /* MSG_COMMS_MPI */])

#########################################
# C compiler
#########################################
AC_MSG_NOTICE
AC_MSG_NOTICE([C compiler])
AC_MSG_NOTICE

ga_save_CFLAGS="$CFLAGS"
GA_PROG_MPICC
CFLAGS="$ga_save_CFLAGS"
AC_USE_SYSTEM_EXTENSIONS
AX_COMPILER_VENDOR
# If F77 is enabled, only perform a simple compilation test.
AS_IF([test "x$with_mpi" != xno],
    [AS_IF([test "x$enable_f77" = xyes],
        [GA_MPICC_TEST_COMPILE],
        [AS_IF([test "x$enable_mpi_tests" = xyes],
            [GA_MPICC_TEST_LINK],
            [GA_MPICC_TEST_COMPILE])])])
# Hack to append .x to executables.
AC_SUBST([EXEEXT], [.x$EXEEXT])

GA_TARGET64

# Establish the underlying network infrastructure (SOCKETS, OPENIB, etc)
GA_ARMCI_NETWORK
# needed for MPI_SPAWN only -- other defines occur in ARMCI configure only
AS_IF([test "x$ga_armci_network" = xMPI_SPAWN],
    [AC_DEFINE([MPI_SPAWN], [1], [defined to 1 when ARMCI_NETWORK=MPI_SPAWN])])

# Checks for C header files.
AC_HEADER_ASSERT
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
GA_CHECK_HEADERS([assert.h errno.h fcntl.h ffio.h float.h inttypes.h limits.h malloc.h math.h memory.h netdb.h netinet/in.h netinet/tcp.h nx.h pfs/pfs.h setjmp.h signal.h stdarg.h stdint.h stdio.h stdlib.h string.h strings.h sys/cnx_types.h sys/errno.h sys/file.h sys/fsid.h sys/fstyp.h sys/ioctl.h sys/iosw.h sys/ipc.h sys/mman.h sys/mount.h sys/param.h sys/select.h sys/sem.h sys/shm.h sys/socket.h sys/stat.h sys/statfs.h sys/statvfs.h sys/systemcfg.h sys/time.h sys/types.h sys/vfs.h sys/wait.h time.h unistd.h windows.h rpc/rpc.h rpc/types.h rpc/xdr.h], [], [],
[[#ifdef HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif]])

# Checks for C typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_VOLATILE
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_TYPE_INT64_T
AC_TYPE_LONG_DOUBLE
AC_TYPE_LONG_LONG_INT
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_CHECK_TYPES([ptrdiff_t])
#AC_CHECK_MEMBERS([struct stat.st_vfstype, struct statfs.f_secnfree, struct statvfs.f_secnfree, struct statfs.f_frsize, struct statvfs.f_frsize])
GA_FUNCTION
GA_PRAGMA_OPERATOR
GA_SYS_WEAK_ALIAS
GA_UNION_SEMUN

# Checks for C type sizes.
AC_CHECK_SIZEOF([void*])
AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AS_IF([test x$ac_cv_type_long_long_int = xyes], [AC_CHECK_SIZEOF([long long])])
AC_CHECK_SIZEOF([float])
AC_CHECK_SIZEOF([double])
AS_IF([test x$ac_cv_type_long_double = xyes], [AC_CHECK_SIZEOF([long double])])

# Checks for C library functions.
AC_FUNC_FORK
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AS_IF([test "x$ga_msg_comms" = xTCGMSG],
    [AC_SEARCH_LIBS([xdr_array], [rpclib], [], [], [])])
AC_SEARCH_LIBS([floor], [m],
    [AC_DEFINE([HAVE_FLOOR], [1],
               [Define to 1 if you have the `floor' function.])],
    [AC_DEFINE([HAVE_FLOOR], [0],
               [Define to 1 if you have the `floor' function.])])
AC_SEARCH_LIBS([modf], [m],
    [AC_DEFINE([HAVE_MODF], [1],
               [Define to 1 if you have the `modf' function.])],
    [AC_DEFINE([HAVE_MODF], [0],
               [Define to 1 if you have the `modf' function.])])
AC_SEARCH_LIBS([pow], [m],
    [AC_DEFINE([HAVE_POW], [1],
               [Define to 1 if you have the `pow' function.])],
    [AC_DEFINE([HAVE_POW], [0],
               [Define to 1 if you have the `pow' function.])])
AC_SEARCH_LIBS([sqrt], [m],
    [AC_DEFINE([HAVE_SQRT], [1],
               [Define to 1 if you have the `sqrt' function.])],
    [AC_DEFINE([HAVE_SQRT], [0],
               [Define to 1 if you have the `sqrt' function.])])
GA_CHECK_FUNCS([bcopy bzero create_shared_region ftruncate gethostbyname gethostname getpagesize gettimeofday memalign memcpy memset munmap pause posix_memalign putenv select setenv shared_malloc shmalloc socket srandom srand strchr strdup strerror strstr strtol xdr_char])

# Checks for C libraries.
GA_CHECK_LIBPLOT

GA_C_OPT

#########################################
# C++ compiler
#########################################
AC_MSG_NOTICE
AS_IF([test "x$enable_cxx" = xyes],
    [AC_MSG_NOTICE([C++ compiler])],
    [AC_MSG_NOTICE([C++ compiler (disabled, but some tests still required)])])
AC_MSG_NOTICE
AC_LANG_PUSH([C++])

# GA_PROG_MPICXX is required to silence complaints that C++ source exists
# even if C++ support has been disabled.
ga_save_CXXFLAGS="$CXXFLAGS"
GA_PROG_MPICXX
CXXFLAGS="$ga_save_CXXFLAGS"

AS_IF([test "x$enable_cxx" = xyes], [

AX_COMPILER_VENDOR
AS_IF([test "x$with_mpi" != xno], [GA_MPICXX_TEST])

# Checks for C++ header files.

# Checks for C++ typedefs, structures, and compiler characteristics.

# Checks for C++ type sizes.

# Checks for C++ library functions.

GA_CXX_OPT

])

AC_LANG_POP([C++])

#########################################
# Assembler
#########################################
AC_MSG_NOTICE
AC_MSG_NOTICE([Assembler])
AC_MSG_NOTICE

AM_PROG_AS

#########################################
# Fortran compiler
#########################################
AC_MSG_NOTICE
AS_IF([test "x$enable_f77" = xyes],
    [AC_MSG_NOTICE([Fortran compiler])],
    [AC_MSG_NOTICE([Fortran compiler (disabled, but some tests still required)])])
AC_MSG_NOTICE
AC_LANG_PUSH([Fortran 77])

ga_save_FFLAGS="$FFLAGS"
GA_PROG_MPIF77
FFLAGS="$ga_save_FFLAGS"

# If Fortran 77 compiler was not found, we disable support for it.
AS_IF([test "x$F77" = x], [enable_f77=no])

AX_COMPILER_VENDOR
# If F77 is enabled, perform a simple link test.
AS_IF([test "x$with_mpi" != xno],
    [AS_IF([test "x$enable_f77" = xyes],
        [AS_IF([test "x$enable_mpi_tests" = xyes],
            [GA_MPIF77_TEST_LINK],
            [GA_MPIF77_TEST_COMPILE])],
        [GA_MPIF77_TEST_COMPILE])])

GA_SUPPRESS_MESSAGE
GA_F77_MAYBE_UNDERSCORING
GA_F77_DUMMY_MAIN
GA_F77_WRAPPERS
GA_F77_CPP_SYMBOL
GA_F77_FIXED
GA_F77_INTEGER_SIZE

# Checks for Fortran typedefs, structures, and compiler characteristics.

# Checks for Fortran type sizes.
ga_save_FFLAGS="$FFLAGS"; FFLAGS="$FFLAGS $FFLAG_INT"
GA_F77_CHECK_SIZEOF([INTEGER], [$ga_cv_f77_integer_size])
FFLAGS="$ga_save_FFLAGS"
GA_F77_CHECK_SIZEOF([REAL], [4])
GA_F77_CHECK_SIZEOF([DOUBLE PRECISION], [8])

# Checks for Fortran library functions.
AS_IF([test "x$enable_f77" = xyes], [
GA_CHECK_FUNCS([dtime etime])
GA_F77_FLUSH
])

# The rest of the Fortran 77 to C compatibility checks.
AS_IF([test "x$enable_f77" = xyes], [
GA_F2C_NOMAIN
GA_F77_OPT
])
GA_F2C_CMDARGS
GA_F2C_HIDDEN_STRING_LENGTH_CONVENTION
GA_F2C_MATCH_TYPES([INTEGER], [short, int, long, long long])
GA_F2C_MATCH_TYPES([REAL], [float, double, long double])
GA_F2C_MATCH_TYPES([DOUBLE PRECISION], [float, double, long double])

# This is required to override what automake does, if needed.
GA_F77_LD_OVERRIDE

AC_LANG_POP([Fortran 77])

###############################################################################
# Checks for BLAS/LAPACK/SCALAPACK require both C and Fortran features to
# have been detected. Further, they assume the current AC_LANG is C.
###############################################################################
GA_BLAS
GA_LAPACK
GA_SCALAPACK

###############################################################################
# Checks for programs.
###############################################################################
AC_PROG_GREP
AC_PROG_SED

AM_PATH_PYTHON([],[],[:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

AC_PATH_PROG([M4], [m4], [notfound])
AM_CONDITIONAL([HAVE_M4], [test "x$M4" != xnotfound])

# X needed for xregion program, but only if desired.
AC_ARG_ENABLE([xregion],
    [AS_HELP_STRING([--enable-xregion], [build global/X/xregion])],
    [enable_xregion=yes],
    [enable_xregion=no])
AS_IF([test "x$enable_xregion" = xyes],
    [AC_PATH_X
     AC_SUBST([X_CPPFLAGS], [-I$x_includes])
     AC_SUBST([X_LDFLAGS], [-L$x_libraries])])
AM_CONDITIONAL([ENABLE_XREGION], [test "x$enable_xregion" = xyes])
AM_CONDITIONAL([NO_X], [test "$no_x" = xyes])

###############################################################################
# Libtool setup -- no compiler/linker tests after this
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([Libtool setup])
AC_MSG_NOTICE
# temporarily restore unwrapped compilers
# this works around a bug where libtool sadly relies on matching compiler
# names in order to determine features (Fortran only, I think)
# libtool doesn't recognize MPI compiler names, nor should it
AS_IF([test x$with_mpi_wrappers = xyes], [
    #AS_IF([test "x$ga_cv_mpicc_naked"  != xerror],[CC="$ga_cv_mpicc_naked"])
    #AS_IF([test "x$ga_cv_mpicxx_naked" != xerror],[CXX="$ga_cv_mpicxx_naked"])
    AS_IF([test "x$ga_cv_mpif77_naked" != xerror],[F77="$ga_cv_mpif77_naked"])
])
LT_INIT([disable-shared])
# and now that that's over, put the MPI compilers back
AS_IF([test x$with_mpi_wrappers = xyes], [
    #CC="$MPICC"
    #CXX="$MPICXX"
    F77="$MPIF77"
])

###############################################################################
# Remaining setup -- some tests, some individual components
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([Miscellaneous remaining setup])
AC_MSG_NOTICE

# Establish the underlying network infrastructure (SOCKETS, OPENIB, etc)
#GA_ARMCI_NETWORK

# Set up TCGMSG.
TCGMSG_REMOTE_SHELL
TCGMSG_ENABLE_TIMINGS

# Set up MA.
MA_ENABLE_ARMCI_MEM_OPTION
MA_LONG_DOUBLE_TYPEDEF
MA_STATS
MA_VERIFY
AS_IF([test "x$ga_cv_target" = xLINUX64],
    [AS_CASE([$host_cpu],
        [x86_64|ppc64],
            [AC_DEFINE([NOUSE_MMAP], [1], [Set some mallopt options])])])

# Create proper types for our access functions.
AS_IF([test "x$ac_cv_sizeof_voidp" = "x8"],
    [ga_access_index_type="integer*8"
     ga_access_index_type_c="long"],
    [ga_access_index_type="integer"
     ga_access_index_type_c="int"])
AC_SUBST([GA_ACCESS_INDEX_TYPE], [$ga_access_index_type])
AC_SUBST([MA_ACCESS_INDEX_TYPE], [$ga_access_index_type])
AC_SUBST([MA_ACCESS_INDEX_TYPE_C], [$ga_access_index_type_c])

AC_CONFIG_FILES([Makefile
                 gaf2c/farg.h
                 gaf2c/typesf2c.h
                 ma/mafdecls.fh
                 ma/matypes.h
                 global/src/global.fh])
AC_CONFIG_SUBDIRS([armci])
AC_OUTPUT

# Report on what we found.
AC_MSG_NOTICE([])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([ $PACKAGE_NAME configured as follows:])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([                TARGET=$ga_cv_target])
AC_MSG_NOTICE([             MSG_COMMS=$ga_msg_comms])
AC_MSG_NOTICE([            GA_MP_LIBS=$GA_MP_LIBS])
AC_MSG_NOTICE([         GA_MP_LDFLAGS=$GA_MP_LDFLAGS])
AC_MSG_NOTICE([        GA_MP_CPPFLAGS=$GA_MP_CPPFLAGS])
AC_MSG_NOTICE([         ARMCI_NETWORK=$ga_armci_network])
AC_MSG_NOTICE([ ARMCI_NETWORK_LDFLAGS=$ARMCI_NETWORK_LDFLAGS])
AC_MSG_NOTICE([    ARMCI_NETWORK_LIBS=$ARMCI_NETWORK_LIBS])
AC_MSG_NOTICE([ARMCI_NETWORK_CPPFLAGS=$ARMCI_NETWORK_CPPFLAGS])
AS_IF([test "x$enable_f77" = xyes], [
AC_MSG_NOTICE([                   F77=$F77])
])
AC_MSG_NOTICE([                    CC=$CC])
AS_IF([test "x$enable_cxx" = xyes], [
AC_MSG_NOTICE([                   CXX=$CXX])
])
AC_MSG_NOTICE([                   CPP=$CPP])
AC_MSG_NOTICE([              CPPFLAGS=$CPPFLAGS])
AS_IF([test "x$enable_f77" = xyes], [
AC_MSG_NOTICE([                FFLAGS=$FFLAGS])
AC_MSG_NOTICE([             FFLAG_INT=$FFLAG_INT])
])
AC_MSG_NOTICE([                CFLAGS=$CFLAGS])
AS_IF([test "x$enable_cxx" = xyes], [
AC_MSG_NOTICE([              CXXFLAGS=$CXXFLAGS])
])
AS_IF([test "x$enable_f77" = xyes], [
AC_MSG_NOTICE([               GA_FOPT=$GA_FOPT])
])
AC_MSG_NOTICE([               GA_COPT=$GA_COPT])
AS_IF([test "x$enable_cxx" = xyes], [
AC_MSG_NOTICE([             GA_CXXOPT=$GA_CXXOPT])
])
AC_MSG_NOTICE([               LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([                  DEFS=$DEFS])
AC_MSG_NOTICE([                  LIBS=$LIBS])
AC_MSG_NOTICE([                 SHELL=$SHELL])
AC_MSG_NOTICE([])