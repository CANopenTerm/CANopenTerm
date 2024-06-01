cmake_minimum_required(VERSION 3.16)
project(CANopenTerm C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# SDL2
set(SDL2_VERSION "2.30.3")

if(WIN32)
  set(SDL2_DEVEL_PKG SDL2-devel-${SDL2_VERSION}-VC.zip)
  set(SDL2_PLATFORM  "x64")

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SDL2_PLATFORM "x86")
  endif()

  set(SDL2_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-${SDL2_VERSION}_${SDL2_PLATFORM})

  ExternalProject_Add(SDL2_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
    URL_HASH SHA1=f1665949752ceabd4d1ba7a547678369487743ad
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL2_PATH}/
    BUILD_BYPRODUCTS ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.lib
    BUILD_BYPRODUCTS ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2main.lib

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build step."

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.dll ${CMAKE_CURRENT_SOURCE_DIR}/export

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL2_devel.cmake" ${SDL2_PATH}/CMakeLists.txt)

  set(SDL2_INCLUDE_DIR ${SDL2_PATH}/include)
  set(SDL2_LIBRARY     ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2.lib)
  set(SDL2MAIN_LIBRARY ${SDL2_PATH}/lib/${SDL2_PLATFORM}/SDL2main.lib)
endif(WIN32)

if(UNIX)
  set(SDL2_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-${SDL2_VERSION})
  set(SDL2_DEVEL_PKG SDL2-${SDL2_VERSION}.tar.gz)

  ExternalProject_Add(SDL2_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
    URL_HASH SHA1=2878b4b1fbe9e4b22a317ad52c9d751c70e8df62
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL2_PATH}/
    BINARY_DIR ${SDL2_PATH}_build
    BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2.a
    BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2main.a

    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
    PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step.")

  set(SDL2_INCLUDE_DIR ${SDL2_PATH}/include)
  set(SDL2_LIBRARY     ${SDL2_PATH}_build/libSDL2.a)
  set(SDL2MAIN_LIBRARY ${SDL2_PATH}_build/libSDL2main.a)
endif(UNIX)

# PCAN-Basic API
set(PCAN_VERSION_WINDOWS "4.6.1")
set(PCAN_VERSION_LINUX   "4.6.1")

if(WIN32)
  set(PCAN_PLATFORM  "x64")
  set(PCAN_PATH      "${CMAKE_CURRENT_SOURCE_DIR}/deps/PCAN-Basic_API_Windows")

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PCAN_PLATFORM "Win32")
  endif()

  set(PCAN_DEVEL_PKG  PCAN-Basic_Windows-${PCAN_VERSION_WINDOWS}.zip)
  set(PCAN_DEVEL_URL  https://canopenterm.de/mirror)
  set(PCAN_DEVEL_HASH 5aa4459340986d921a63f15cc643733ab7d9c011)

  ExternalProject_Add(PCAN_devel
    URL ${PCAN_DEVEL_URL}/${PCAN_DEVEL_PKG}
    URL_HASH SHA1=${PCAN_DEVEL_HASH}
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${PCAN_PATH}/
    BUILD_BYPRODUCTS ${PCAN_PATH}/${PCAN_PLATFORM}/VC_LIB/PCANBasic.lib

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build step."

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${PCAN_PATH}/${PCAN_PLATFORM}/PCANBasic.dll ${CMAKE_CURRENT_SOURCE_DIR}/export

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PCAN_devel.cmake" ${PCAN_PATH}/CMakeLists.txt)

  set(PCAN_INCLUDE_DIR ${PCAN_PATH}/Include)
  set(PCAN_LIBRARY     ${PCAN_PATH}/${PCAN_PLATFORM}/VC_LIB/PCANBasic.lib)
endif(WIN32)

if(UNIX)
  set(PCAN_PATH      "${CMAKE_CURRENT_SOURCE_DIR}/deps/PCAN-Basic_API_Linux")
  set(PCAN_DEVEL_PKG PCAN-Basic_Linux-${PCAN_VERSION_LINUX}.tar.gz)
  set(PCAN_DEVEL_URL https://canopenterm.de/mirror)
  set(PCAN_DEVEL_HASH fa7f9eb4635497036b8dedef6e8eb27e3e435d7f)

  ExternalProject_Add(PCAN_devel
    URL ${PCAN_DEVEL_URL}/${PCAN_DEVEL_PKG}
    URL_HASH SHA1=${PCAN_DEVEL_HASH}
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${PCAN_PATH}/

    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PCAN_devel.cmake" ${PCAN_PATH}/CMakeLists.txt)

  set(PCAN_INCLUDE_DIR
    ${PCAN_PATH}/libpcanbasic/pcanbasic)

  set(PCAN_LIBRARY pcanbasic)
  link_directories(${PCAN_PATH})
endif(UNIX)

# libsocketcan
#if(UNIX)
#  set(SOCKETCAN_VERSION   "0.0.12")
#  set(SOCKETCAN_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/libsocketcan-${SOCKETCAN_VERSION})
#  set(SOCKETCAN_DEVEL_PKG libsocketcan-${SOCKETCAN_VERSION}.tar.gz)

#  ExternalProject_Add(socketcan_devel
#    URL https://git.pengutronix.de/cgit/tools/libsocketcan/snapshot/${SOCKETCAN_DEVEL_PKG}
#    URL_HASH SHA1=b914fc7e32184f8a43bff79723a17d426a2e1037
#    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
#    DOWNLOAD_NO_PROGRESS true
#    TLS_VERIFY true
#    SOURCE_DIR ${SOCKETCAN_PATH}/

#    BUILD_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping build step."
#    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

#    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/socketcan_devel.cmake" ${SOCKETCAN_PATH}/CMakeLists.txt)

#  set(SOCKETCAN_INCLUDE_DIR ${SOCKETCAN_PATH}/include)
#endif(UNIX)

# Nuklear
set(NUKLEAR_VERSION   "4.12.0")
set(NUKLEAR_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/Nuklear-${NUKLEAR_VERSION})
set(NUKLEAR_DEVEL_PKG ${NUKLEAR_VERSION}.zip)

ExternalProject_Add(Nuklear_devel
  URL https://github.com/Immediate-Mode-UI/Nuklear/archive/refs/tags/${NUKLEAR_DEVEL_PKG}
  URL_HASH SHA1=3eb18d3b1d9424d5e4a8a1dee2d053417e1b9e20
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${NUKLEAR_PATH}/

  BUILD_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping build step."
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Nuklear_devel.cmake" ${NUKLEAR_PATH}/CMakeLists.txt)

set(NUKLEAR_INCLUDE_DIR ${NUKLEAR_PATH})

# Lua
set(LUA_VERSION   "5.4.6")
set(LUA_DEVEL_PKG v${LUA_VERSION}.zip)

if(WIN32)
  set(LUA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-${LUA_VERSION}_Windows)
endif(WIN32)

if(UNIX)
  set(LUA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-${LUA_VERSION}_Linux)
endif(UNIX)

ExternalProject_Add(Lua_devel
  URL https://github.com/lua/lua/archive/refs/tags/${LUA_DEVEL_PKG}
  URL_HASH SHA1=96abb80f46e2c6548b47632384205bddfaeb6c37
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${LUA_PATH}/

  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Lua_devel.cmake" ${LUA_PATH}/CMakeLists.txt)

set(LUA_INCLUDE_DIR ${LUA_PATH})
link_directories(${LUA_PATH})

# dirent
if(WIN32)
  set(DIRENT_VERSION   "1.2.4")
  set(DIRENT_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/dirent-${DIRENT_VERSION})
  set(DIRENT_DEVEL_PKG ${DIRENT_VERSION}.zip)

  ExternalProject_Add(dirent_devel
    URL https://github.com/tronkko/dirent/archive/refs/tags/${DIRENT_DEVEL_PKG}
    URL_HASH SHA1=70b02369071572dd1b080057a6b9170dec04868d
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${DIRENT_PATH}/

    BUILD_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping build step."
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dirent_devel.cmake" ${DIRENT_PATH}/CMakeLists.txt)

  set(DIRENT_INCLUDE_DIR ${DIRENT_PATH}/include)
endif()

include_directories(
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  SYSTEM ${SDL2_INCLUDE_DIR}
  SYSTEM ${PCAN_INCLUDE_DIR}
  SYSTEM ${PCAN_INCLUDE_DIR}/src/pcan/driver
  SYSTEM ${PCAN_INCLUDE_DIR}/src/pcan/lib
#  SYSTEM ${SOCKETCAN_INCLUDE_DIR}
  SYSTEM ${NUKLEAR_INCLUDE_DIR}
  SYSTEM ${LUA_INCLUDE_DIR}
  SYSTEM ${DIRENT_INCLUDE_DIR})

set(project_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/can.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/menu_bar.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/nmt_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pdo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/printf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdo_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/table.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/export)

add_executable(
  ${PROJECT_NAME}
  ${GUI_TYPE}
  ${project_sources})

add_dependencies(
  ${PROJECT_NAME}
  SDL2_devel
  PCAN_devel
  Nuklear_devel
  Lua_devel)

#if(UNIX)
#  add_dependencies(
#    ${PROJECT_NAME}
#    socketcan_devel)
#endif(UNIX)

if(WIN32)
  target_link_libraries(
    ${PROJECT_NAME}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${PCAN_LIBRARY}
    lua)
endif(WIN32)

if(UNIX)
  target_link_libraries(
    ${PROJECT_NAME}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${PCAN_LIBRARY}
    lua
    dl
    m
    pthread)
endif(UNIX)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(COMPILE_OPTIONS
    /W4
    /utf-8)
endif()
