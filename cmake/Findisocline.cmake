cmake_minimum_required(VERSION 3.16)

# Findisocline.cmake
# Locate isocline library and include directories

find_path(isocline_INCLUDE_DIR
    NAMES isocline.h
)

find_library(isocline_LIBRARY
    NAMES isocline
)

message(STATUS "isocline_INCLUDE_DIR: ${isocline_INCLUDE_DIR}")
message(STATUS "isocline_LIBRARY: ${isocline_LIBRARY}")

if (isocline_INCLUDE_DIR AND isocline_LIBRARY)
  set(isocline_FOUND TRUE)
  set(isocline_INCLUDE_DIRS ${isocline_INCLUDE_DIR})
  set(isocline_LIBRARIES ${isocline_LIBRARY})
else()
  set(isocline_FOUND FALSE)
endif()

mark_as_advanced(isocline_INCLUDE_DIR isocline_LIBRARY)
