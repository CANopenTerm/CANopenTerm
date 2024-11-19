# Findpocketpy.cmake
# Locate pocketpy library and include directories

find_path(pocketpy_INCLUDE_DIR NAMES pocketpy.h)
find_library(pocketpy_LIBRARY NAMES pocketpy)

if (pocketpy_INCLUDE_DIR AND pocketpy_LIBRARY)
    set(pocketpy_FOUND TRUE)
    set(pocketpy_INCLUDE_DIRS ${pocketpy_INCLUDE_DIR})
    set(pocketpy_LIBRARIES ${pocketpy_LIBRARY})
else()
    set(pocketpy_FOUND FALSE)
endif()

mark_as_advanced(pocketpy_INCLUDE_DIR pocketpy_LIBRARY)
