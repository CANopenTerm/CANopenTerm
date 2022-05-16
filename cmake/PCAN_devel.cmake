cmake_minimum_required(VERSION 3.10)
project(PCAN_devel C)

if(UNIX)
  set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libpcanbasic/pcanbasic/src)

  set(pcanbasic_sources
    ${SRC_DIR}/libpcanbasic.c
    ${SRC_DIR}/pcaninfo.c
    ${SRC_DIR}/pcanlog.c
    ${SRC_DIR}/pcbcore.c
    ${SRC_DIR}/pcblog.c
    ${SRC_DIR}/pcbtrace.c
    ${SRC_DIR}/pcan/lib/src/libpcanfd.c)

  add_library(pcanbasic
    STATIC
    ${pcanbasic_sources})

  target_include_directories(
    pcanbasic
    PUBLIC ${SRC_DIR}/pcan/driver
    PUBLIC ${SRC_DIR}/pcan/lib)

  target_compile_definitions(
    pcanbasic
    PUBLIC
    NO_RT)
endif(UNIX)
