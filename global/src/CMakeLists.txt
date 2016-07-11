#
# module: CMakeLists.txt
# author: Bruce Palmer
# description: implements a primative CMake build that can be used to build
#              GA on Windows-based systems. Only MPI-based runtimes are
#              supported.
# 
# DISCLAIMER
#
# This material was prepared as an account of work sponsored by an
# agency of the United States Government.  Neither the United States
# Government nor the United States Department of Energy, nor Battelle,
# nor any of their employees, MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
# ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE ACCURACY,
# COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT,
# SOFTWARE, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT
# INFRINGE PRIVATELY OWNED RIGHTS.
#
#
# ACKNOWLEDGMENT
#
# This software and its documentation were produced with United States
# Government support under Contract Number DE-AC06-76RLO-1830 awarded by
# the United States Department of Energy.  The United States Government
# retains a paid-up non-exclusive, irrevocable worldwide license to
# reproduce, prepare derivative works, perform publicly and display
# publicly by or for the US Government, including the right to
# distribute to other US Government contractors.
#
# -*- mode: cmake -*-
# -------------------------------------------------------------
# file: CMakeLists.txt
# -------------------------------------------------------------

if (ENABLE_FORTRAN)
  set(GA_FORTRAN_INTERFACE_C_FILES
#fapi.c
  )
  set(GA_FORTRAN_INTERFACE_H_FILES
#cnames.h
  )
endif()

set(target_libraries
    ma
    elio
    dra
    eaf
    sf)

set(WAPI_FILES
  ga-wapi.h
  ga-wapidefs.h
)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/gaf2c
                    ${PROJECT_SOURCE_DIR}/ma
                    ${PROJECT_SOURCE_DIR}/comex/src-armci
                    ${PROJECT_SOURCE_DIR}/tcgmsg
                    ${PROJECT_SOURCE_DIR}/LinAlg/lapack+blas
)

set(GA_FILES
  base.c
  onesided.c
  collect.c
  ghosts.c
  capi.c
  datatypes.c
  decomp.c
  DP.c
  elem_alg.c
  ga_diag_seqc.c
  ga_malloc.c
  ga_profile.c
  ga_solve_seq.c
  ga_symmetr.c
  ga_trace.c
  global.nalg.c
  global.npatch.c
  global.periodic.c
  global.util.c
  hsort.scat.c
  matmul.c
  matrix.c
  nbutil.c
  peigstubs.c
  sclstubs.c
  select.c
  sparse.c
  ${GA_FORTRAN_INTERFACE_C_FILES}
)

add_library(ga_src OBJECT
  ${GA_FILES}
)

add_custom_target(machine_files
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR} ga-papi.h
)

add_custom_command(
  TARGET machine_files
  DEPENDS ga-papi.h
  COMMAND ${PROJECT_SOURCE_DIR}/tools/ga_papi_to_wapi.pl < ${CMAKE_CURRENT_SOURCE_DIR}/ga-papi.h >
${CMAKE_CURRENT_SOURCE_DIR}/ga-wapi.h
)

add_custom_command(
  TARGET machine_files
  DEPENDS ga-papi.h
  COMMAND ${PROJECT_SOURCE_DIR}/tools/ga_papi_to_wapidefs.pl < ${CMAKE_CURRENT_SOURCE_DIR}/ga-papi.h >
${CMAKE_CURRENT_SOURCE_DIR}/ga-wapidefs.h
)
  
add_dependencies(ga machine_files)

# -------------------------------------------------------------
# Global Arrays header installation
# -------------------------------------------------------------
install(FILES
#  abstract_ops.h
#  base.h
#  cnames.h
  gacommon.h
#  gaconfig.h
  ga.h
  ga-mpi.h
  ga-papi.h
#  ga_profile.h
#  ga-wapidefs.h
  ga-wapi.h
#  global.h
#  globalp.h
#  matmul.h
  ${GA_FORTRAN_INTERFACE_H_FILES}
  DESTINATION include
)