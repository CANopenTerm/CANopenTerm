cmake_minimum_required(VERSION 3.16)

# Findinih.cmake
# Locate inih library and include directories

find_path(inih_INCLUDE_DIR NAMES ini.h)
find_library(inih_LIBRARY NAMES inih)

if (inih_INCLUDE_DIR AND inih_LIBRARY)
    set(inih_FOUND TRUE)
    set(inih_INCLUDE_DIRS ${inih_INCLUDE_DIR})
    set(inih_LIBRARIES ${inih_LIBRARY})
else()
    set(inih_FOUND FALSE)
endif()

mark_as_advanced(inih_INCLUDE_DIR inih_LIBRARY)
