cmake_minimum_required(VERSION 3.17)
project(CANopenTerm C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# SDL2
set(SDL2_VERSION "2.0.22")

if (WIN32)
  #if (CMAKE_BUILD_TYPE MATCHES Release)
  #  set(GUI_TYPE WIN32)
  #endif()

  set(SDL2_DEVEL_PKG SDL2-devel-${SDL2_VERSION}-VC.zip)
  set(SDL2_PLATFORM  "x64")

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SDL2_PLATFORM "x86")
  endif()

  set(SDL2_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-${SDL2_VERSION}_${SDL2_PLATFORM})

  ExternalProject_Add(SDL2_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
    URL_HASH SHA1=efa040633c4faf8b006c0c1e552456ca4e5a3a53
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
    URL_HASH SHA1=66dd460befa7f4c4664b7591228a3951bbbb19a7
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
if(WIN32)
  set(PCAN_PLATFORM  "x64")
  set(PCAN_PATH      "${CMAKE_CURRENT_SOURCE_DIR}/deps/PCAN-Basic_API_Windows")

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PCAN_PLATFORM "Win32")
  endif()

  set(PCAN_DEVEL_PKG  pcan-basic.zip)
  set(PCAN_DEVEL_URL  https://www.peak-system.com/fileadmin/media/files)
  set(PCAN_DEVEL_HASH 02ade3d72dfe1e5611f378b5d58fd009ed20319d)

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
  set(PCAN_DEVEL_PKG PCAN-Basic_Linux-4.6.0.tar.gz)
  set(PCAN_DEVEL_URL https://www.peak-system.com/produktcd/Develop/PC%20interfaces/Linux/PCAN-Basic_API_for_Linux)
  set(PCAN_DEVEL_HASH 348db1fd0c685a88fce846e94fd8ddac277fe838)

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

# Nuklear
set(NUKLEAR_VERSION   "4.9.6")
set(NUKLEAR_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/Nuklear-${NUKLEAR_VERSION})
set(NUKLEAR_DEVEL_PKG ${NUKLEAR_VERSION}.zip)

ExternalProject_Add(Nuklear_devel
  URL https://github.com/Immediate-Mode-UI/Nuklear/archive/refs/tags/${NUKLEAR_DEVEL_PKG}
  URL_HASH SHA1=d7e3273bfe27634cfc1b92e86316e5af53b407eb
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
set(LUA_VERSION   "5.4.4")
set(LUA_DEVEL_PKG v${LUA_VERSION}.zip)

if(WIN32)
  set(LUA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-${LUA_VERSION}_Windows)
endif(WIN32)

if(UNIX)
  set(LUA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-${LUA_VERSION}_Linux)
endif(UNIX)

ExternalProject_Add(Lua_devel
  URL https://github.com/lua/lua/archive/refs/tags/${LUA_DEVEL_PKG}
  URL_HASH SHA1=21b2815c0ec280877a2a8af4b865cba30df80260
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
  set(DIRENT_VERSION   "1.23.2")
  set(DIRENT_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/dirent-${DIRENT_VERSION})
  set(DIRENT_DEVEL_PKG ${DIRENT_VERSION}.zip)

  ExternalProject_Add(dirent_devel
    URL https://github.com/tronkko/dirent/archive/refs/tags/${DIRENT_DEVEL_PKG}
    URL_HASH SHA1=0059b1031f4cd078ab0f9bfcadc3903949e4dd3d
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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/printf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdo_client.c)

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
