cmake_minimum_required(VERSION 3.16)
project(CANopenTerm C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/export)

option(BUILD_TESTS             "Build CMocka unit tests"              OFF)
option(ENABLE_POCKETPY_SUPPORT "Enable experimental pocketpy support" OFF)

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
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/os_${PLATFORM}.cmake)

if(BUILD_TESTS)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests.cmake)
endif(BUILD_TESTS)

include_directories(
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/os)

function(append_platform_files source_list pattern platform)
  file(GLOB platform_files ${pattern}_${platform}.c)
  list(APPEND ${source_list} ${platform_files})
  set(${source_list} ${${source_list}} PARENT_SCOPE)
endfunction()

set(common_core_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/buffer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/can.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/command.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/dbc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/dict.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/nmt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/pdo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/scripts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/sdo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/table.c
)

set(common_os_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/os/os.c)

set(core_sources ${common_core_sources})
set(os_sources ${common_os_sources})

append_platform_files(core_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*" ${PLATFORM})
append_platform_files(os_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/os/*" ${PLATFORM})

add_library(
  core
  STATIC
  ${core_sources}
  ${os_sources})

target_link_libraries(
  ${PROJECT_NAME}
  core
  ${PLATFORM_LIBS})

add_dependencies(
  ${PROJECT_NAME}
  core)

add_dependencies(
  core
  ${PLATFORM_CORE_DEPS})

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
