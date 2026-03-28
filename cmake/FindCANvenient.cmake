cmake_minimum_required(VERSION 3.16)

# FindCANvenient.cmake
# Locate CANvenient library and include directories

find_path(CANvenient_INCLUDE_DIR NAMES CANvenient.h HINTS /usr/include)
find_library(CANvenient_LIBRARY NAMES CANvenient HINTS /usr/lib)

if (CANvenient_INCLUDE_DIR AND CANvenient_LIBRARY)
  set(CANvenient_FOUND TRUE)
  set(CANvenient_INCLUDE_DIRS ${CANvenient_INCLUDE_DIR})
  set(CANvenient_LIBRARIES ${CANvenient_LIBRARY})
else()
  set(CANvenient_FOUND FALSE)
endif()

mark_as_advanced(CANvenient_INCLUDE_DIR CANvenient_LIBRARY)
