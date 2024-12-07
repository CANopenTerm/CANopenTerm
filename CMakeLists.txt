cmake_minimum_required(VERSION 3.16)
project(CANopenTerm C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/export)

option(BUILD_TESTS              "Build CMocka unit tests"              OFF)
option(BUILD_YOCTO              "Build in a Yocto build environment"   OFF)
option(ENABLE_POCKETPY_SUPPORT  "Enable experimental pocketpy support" OFF)
option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer (Clang)"     OFF)
option(USE_VALGRIND             "Build with Valgrind support"          OFF)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

add_executable(
    ${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c ${CMAKE_CURRENT_SOURCE_DIR}/src/icon.rc)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

if(WIN32)
    set(PLATFORM "windows")
elseif(UNIX)
    set(PLATFORM "linux")

    if (USE_VALGRIND)
        add_compile_definitions(USE_VALGRIND)
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/os_${PLATFORM}.cmake)

if(BUILD_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests.cmake)
    include(CTest)
    enable_testing()
    add_test(NAME run_unit_tests COMMAND run_unit_tests)
    set_tests_properties(run_unit_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export)
endif(BUILD_TESTS)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/codb2json.cmake)

include_directories(
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/api
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/codb2json
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/os)

function(append_platform_files source_list pattern platform)
    file(GLOB platform_files ${pattern}_${platform}.c)
    list(APPEND ${source_list} ${platform_files})
    set(${source_list} ${${source_list}} PARENT_SCOPE)
endfunction()

set(api_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_dbc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_nmt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_pdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/lua_test_report.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_dbc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_nmt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_pdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/python_test_report.c)

set(common_core_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/command.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/codb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/dbc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/dict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/eds.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/nmt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/pdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/scripts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/sdo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/test_report.c)

set(common_os_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/os.c)

set(core_sources ${common_core_sources})
set(os_sources ${common_os_sources})

append_platform_files(core_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*" ${PLATFORM})
append_platform_files(os_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/os/*" ${PLATFORM})

add_library(
    core
    STATIC
    ${api_sources}
    ${core_sources}
    ${os_sources})

target_link_libraries(
    ${PROJECT_NAME}
    core
    ${PLATFORM_LIBS})

add_dependencies(
    ${PROJECT_NAME}
    core)

if(NOT BUILD_YOCTO)
    add_dependencies(
        core
        ${PLATFORM_CORE_DEPS})
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(
        ${PROJECT_NAME}
        PUBLIC
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unsafe-buffer-usage
        -Wno-covered-switch-default
        -Werror)

    if (ENABLE_ADDRESS_SANITIZER)
        target_compile_options(
            ${PROJECT_NAME}
            PUBLIC
            -fsanitize=address)

        target_link_options(
            ${PROJECT_NAME}
            PUBLIC
            -fsanitize=address)

        target_link_libraries(
            ${PROJECT_NAME}
            clang_rt.asan_dynamic-x86_64
            clang_rt.asan_dynamic_runtime_thunk-x86_64)

    endif()

elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
        ${PROJECT_NAME}
        PUBLIC
        -Wall
        -Wextra
        -Wpedantic)

elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(
        ${PROJECT_NAME}
        PUBLIC
        /W4
        /utf-8)

    set(MSVC_REDIST_DIRS
        "C:/Program Files/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2019/Community/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Redist/MSVC"
    )

    set(MSVC_REDIST_DIR "")
    foreach(DIR ${MSVC_REDIST_DIRS})
        file(GLOB_RECURSE FOUND_DLL "${DIR}/*/x64/Microsoft.VC143.CRT/vcruntime140.dll")
        if (FOUND_DLL)
            get_filename_component(MSVC_REDIST_DIR ${FOUND_DLL} DIRECTORY)
            break()
        endif()
    endforeach()

    if (MSVC_REDIST_DIR)
        message(STATUS "Found vcruntime140.dll at ${MSVC_REDIST_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSVC_REDIST_DIR}/vcruntime140.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    else()
        message(WARNING "vcruntime140.dll not found. Please ensure the Visual Studio redistributable is installed.")
    endif()

endif()
