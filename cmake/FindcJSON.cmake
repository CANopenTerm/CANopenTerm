# FindcJSON.cmake
# Locate cJSON library and include directories

find_path(CJSON_INCLUDE_DIR NAMES cJSON.h HINTS /usr/include/cjson)
message(STATUS "CJSON_INCLUDE_DIR: ${CJSON_INCLUDE_DIR}")
find_library(CJSON_LIBRARY NAMES cjson HINTS /usr/lib)
message(STATUS "CJSON_LIBRARY: ${CJSON_LIBRARY}")

if (CJSON_INCLUDE_DIR AND CJSON_LIBRARY)
    set(cJSON_FOUND TRUE)
    set(cJSON_INCLUDE_DIRS ${CJSON_INCLUDE_DIR})
    set(cJSON_LIBRARIES ${CJSON_LIBRARY})
else()
    set(cJSON_FOUND FALSE)
endif()

mark_as_advanced(CJSON_INCLUDE_DIR CJSON_LIBRARY)
