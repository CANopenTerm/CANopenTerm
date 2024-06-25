cmake_minimum_required(VERSION 3.16)
project(CANopenTerm C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/export)

option(UNIT_TESTING "Build with CMocka unit tests" True)

set(CMAKE_C_STANDARD 90)
if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

add_executable(
  ${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

if(WIN32)
  set(PLATFORM "windows")
elseif(UNIX)
  set(PLATFORM "linux")
endif()

# Lua
set(LUA_VERSION   "5.4.6")
set(LUA_DEVEL_PKG v${LUA_VERSION}.zip)
set(LUA_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua-${LUA_VERSION}_${PLATFORM})

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
find_library(LUA_LIBRARY NAMES lua PATHS ${LUA_PATH})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/os_${PLATFORM}.cmake)

if(UNIT_TESTING)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/unit_tests.cmake)
endif(UNIT_TESTING)

include_directories(
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/os
  SYSTEM ${SDL2_INCLUDE_DIR}
  SYSTEM ${PCAN_INCLUDE_DIR}
  SYSTEM ${PCAN_INCLUDE_DIR}/../src/pcan/driver
  SYSTEM ${PCAN_INCLUDE_DIR}/../src/pcan/lib
  SYSTEM ${LUA_INCLUDE_DIR}
  SYSTEM ${DIRENT_INCLUDE_DIR})

set(core_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/can.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dict.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/nmt_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pdo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sdo_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/os/os.c)

add_library(
  core
  STATIC
  ${core_sources}
  ${os_sources})

if(PLATFORM_DEPS)
  add_dependencies(
    ${PROJECT_NAME}
    ${PLATFORM_DEPS})
endif(PLATFORM_DEPS)

add_dependencies(
  core
  SDL2_devel
  Lua_devel)

add_dependencies(
  ${PROJECT_NAME}
  core)

target_link_libraries(
  ${PROJECT_NAME}
  core
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  ${LUA_LIBRARY})

if(PLATFORM_LIBS)
  target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})
endif(PLATFORM_LIBS)

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
