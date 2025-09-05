cmake_minimum_required(VERSION 3.16)

# Findpocketpy.cmake
# Locate pocketpy library and include directories

find_path(pocketpy_INCLUDE_DIR NAMES pocketpy/pocketpy.h PATHS /usr/include)
find_library(pocketpy_LIBRARY NAMES libpocketpy.so PATHS /usr/lib)

message(STATUS "pocketpy_INCLUDE_DIR: ${pocketpy_INCLUDE_DIR}")
message(STATUS "pocketpy_LIBRARY: ${pocketpy_LIBRARY}")

if (pocketpy_INCLUDE_DIR AND pocketpy_LIBRARY)
  set(pocketpy_FOUND TRUE)
  set(pocketpy_INCLUDE_DIRS ${pocketpy_INCLUDE_DIR}/pocketpy)
  set(pocketpy_LIBRARIES ${pocketpy_LIBRARY})
else()
  set(pocketpy_FOUND FALSE)
endif()

mark_as_advanced(pocketpy_INCLUDE_DIR pocketpy_LIBRARY)
