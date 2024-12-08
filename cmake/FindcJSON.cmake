# FindcJSON.cmake
# Locate cJSON library and include directories

find_path(CJSON_INCLUDE_DIRS NAMES cJSON.h)
message(STATUS "cJSON_INCLUDE_DIR: ${cJSON_INCLUDE_DIR}")
find_library(CJSON_LIBRARY NAMES cjson HINTS /usr/lib)
message(STATUS "cJSON_LIBRARY: ${cJSON_LIBRARY}")

if (CJSON_INCLUDE_DIR AND CJSON_LIBRARY)
    set(cJSON_FOUND TRUE)
    set(cJSON_INCLUDE_DIRS ${CJSON_INCLUDE_DIR})
    set(cJSON_LIBRARIES ${CJSON_LIBRARY})
else()
    set(cJSON_FOUND FALSE)
endif()

mark_as_advanced(cJSON_INCLUDE_DIR cJSON_LIBRARY)
