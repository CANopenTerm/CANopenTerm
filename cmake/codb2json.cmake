cmake_minimum_required(VERSION 3.10)

add_executable(
    codb2json
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codb2json/codb2json.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codb2json/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/icon.rc)

target_link_libraries(
    codb2json
    ${CJSON_LIBRARY}
    ${SDL3_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${PLATFORM_LIBS})

if(NOT BUILD_YOCTO)
    add_dependencies(
        codb2json
        cJSON_devel
        SDL3_devel)
endif()
