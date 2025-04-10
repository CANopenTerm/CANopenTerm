cmake_minimum_required(VERSION 3.16)

# FindcJSON.cmake
# Locate cJSON library and include directories

find_path(cJSON_INCLUDE_DIR NAMES cJSON.h HINTS /usr/include/cjson)
find_library(cJSON_LIBRARY NAMES cjson HINTS /usr/lib)

if (cJSON_INCLUDE_DIR AND cJSON_LIBRARY)
    set(cJSON_FOUND TRUE)
    set(cJSON_INCLUDE_DIRS ${cJSON_INCLUDE_DIR})
    set(cJSON_LIBRARIES ${cJSON_LIBRARY})
else()
    set(cJSON_FOUND FALSE)
endif()

mark_as_advanced(cJSON_INCLUDE_DIR cJSON_LIBRARY)
