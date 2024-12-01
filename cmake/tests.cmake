add_executable(
    run_unit_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/run_unit_tests.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_codb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_dict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_nmt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_scripts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_wrapper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codb2json/codb2json.c)

add_dependencies(
    run_unit_tests
    core
    cJSON_devel
    CMocka_devel
    SDL2_devel
    Lua_devel)

target_link_libraries(
    run_unit_tests
    core
    ${CMocka_LIBRARY}
    ${CJSON_LIBRARY}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${LUA_LIBRARY}
    ${PLATFORM_LIBS})

add_link_options(
    -Wl,--wrap=can_read
    -Wl,--wrap=can_write)

include_directories(
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    SYSTEM ${CMocka_INCLUDE_DIR}
    SYSTEM ${SDL2_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR})
