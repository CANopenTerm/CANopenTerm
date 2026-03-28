cmake_minimum_required(VERSION 3.16)

# FindCANvenient.cmake

find_path(CANvenient_INCLUDE_DIR 
  NAMES CANvenient.h
  PATH_SUFFIXES include
)

find_library(CANvenient_LIBRARY 
  NAMES CANvenient
  PATH_SUFFIXES lib lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CANvenient
  REQUIRED_VARS CANvenient_LIBRARY CANvenient_INCLUDE_DIR
)

if(CANvenient_FOUND)
  set(CANvenient_INCLUDE_DIRS ${CANvenient_INCLUDE_DIR})
  set(CANvenient_LIBRARIES ${CANvenient_LIBRARY})

  if(NOT TARGET CANvenient::CANvenient)
    add_library(CANvenient::CANvenient UNKNOWN IMPORTED)
    set_target_properties(CANvenient::CANvenient PROPERTIES
      IMPORTED_LOCATION "${CANvenient_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${CANvenient_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(CANvenient_INCLUDE_DIR CANvenient_LIBRARY)
