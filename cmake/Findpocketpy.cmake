cmake_minimum_required(VERSION 3.16)

# Findpocketpy.cmake
# Locate pocketpy library and include directories

find_path(pocketpy_INCLUDE_DIR
  NAMES pocketpy.h
  PATH_SUFFIXES pocketpy
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /opt/homebrew/include
)

find_library(pocketpy_LIBRARY
  NAMES pocketpy
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /opt/homebrew/lib
)

message(STATUS "pocketpy_INCLUDE_DIR: ${pocketpy_INCLUDE_DIR}")
message(STATUS "pocketpy_LIBRARY: ${pocketpy_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pocketpy
  REQUIRED_VARS pocketpy_LIBRARY pocketpy_INCLUDE_DIR
)

if (pocketpy_FOUND)
  set(pocketpy_INCLUDE_DIRS ${pocketpy_INCLUDE_DIR})
  set(pocketpy_LIBRARIES ${pocketpy_LIBRARY})
endif()

mark_as_advanced(pocketpy_INCLUDE_DIR pocketpy_LIBRARY)
