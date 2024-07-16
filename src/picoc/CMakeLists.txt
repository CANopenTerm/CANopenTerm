cmake_minimum_required(VERSION 3.10)
project(picoc C)

set(picoc_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/clibrary.c
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/expression.c
  ${CMAKE_CURRENT_SOURCE_DIR}/heap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/parse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/platform.c
  ${CMAKE_CURRENT_SOURCE_DIR}/table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/type.c
  ${CMAKE_CURRENT_SOURCE_DIR}/variable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/ctype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/errno.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/math.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/stdbool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/stdio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/stdlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/string.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cstdlib/time.c)

if(WIN32)

set(picoc_sources
  ${picoc_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/library_msvc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_msvc.c)

elseif(UNIX)

set(picoc_sources
  ${picoc_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/library_unix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_unix.c)

endif()

add_library(picoc
  STATIC
  ${picoc_sources})

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

target_include_directories(picoc
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
