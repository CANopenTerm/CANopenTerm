cmake_minimum_required(VERSION 3.16)
project(Lua_devel C)

set(lua_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lapi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lauxlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lbaselib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lcode.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lcorolib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lctype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ldblib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ldebug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ldo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ldump.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lfunc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lgc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/linit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/liolib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/llex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lmathlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/loadlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lobject.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lopcodes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/loslib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lparser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lstate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lstring.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lstrlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ltable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ltablib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ltm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lundump.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lutf8lib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lvm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lzio.c
)

add_library(lua
  STATIC
  ${lua_sources}
)

if(UNIX)
  target_compile_definitions(lua
    PUBLIC
    LUA_USE_C89
    LUA_USE_LINUX
  )
endif()

set_target_properties(lua
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}
  ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}
  ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR}
)
