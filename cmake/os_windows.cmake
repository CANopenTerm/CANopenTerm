cmake_minimum_required(VERSION 3.16)

if(BUILD_TESTS)
    # CMocka
    set(CMocka_VERSION     "1.1.7")
    set(CMocka_DEVEL_PKG   cmocka-${CMocka_VERSION}.tar.xz)
    set(CMocka_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${CMocka_VERSION})
    set(CMocka_BUILD_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${CMocka_VERSION}_build)
    set(CMocka_INCLUDE_DIR ${CMocka_PATH}/include)
    set(CMocka_LIBRARY     ${CMocka_BUILD_PATH}/src/cmocka.lib)

    ExternalProject_Add(CMocka_devel
        URL https://cmocka.org/files/1.1/${CMocka_DEVEL_PKG}
        URL_HASH SHA1=04cf44545a22e7182803a092a30af5c1a42c31bc
        DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
        DOWNLOAD_NO_PROGRESS true
        TLS_VERIFY true
        SOURCE_DIR ${CMocka_PATH}
        BINARY_DIR ${CMocka_BUILD_PATH}
        BUILD_BYPRODUCTS ${CMocka_LIBRARY}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
            -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON
            -DBUILD_SHARED_LIBS=ON
            -DWITH_EXAMPLES=OFF

        PATCH_COMMAND ${CMAKE_COMMAND} -DCMocka_PATH=${CMocka_PATH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_cmocka.cmake

        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
            ${CMocka_PATH}_build/src/cmocka.dll ${CMAKE_CURRENT_SOURCE_DIR}/export)
endif(BUILD_TESTS)

# cJSON
set(CJSON_VERSION     "1.7.18")
set(CJSON_DEVEL_PKG   "v${CJSON_VERSION}.zip")
set(CJSON_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/CJSON-${CJSON_VERSION})
set(CJSON_INCLUDE_DIR ${CJSON_PATH})
set(CJSON_LIBRARY     ${CJSON_PATH}_build/cjson.lib)

ExternalProject_Add(cJSON_devel
    URL https://github.com/DaveGamble/cJSON/archive/refs/tags/${CJSON_DEVEL_PKG}
    URL_HASH SHA1=111ea457d2dbe15116e2a8ccbfe557e047040108
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${CJSON_PATH}/
    BINARY_DIR ${CJSON_PATH}_build/
    BUILD_BYPRODUCTS ${CJSON_LIBRARY}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy ${CJSON_PATH}_build/cjson.dll ${CMAKE_CURRENT_SOURCE_DIR}/export)

# pocketpy
set(POCKETPY_VERSION     "2.0.8")
set(POCKETPY_DEVEL_PKG   "v${POCKETPY_VERSION}.zip")
set(POCKETPY_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/pocketpy-${POCKETPY_VERSION})
set(POCKETPY_INCLUDE_DIR ${POCKETPY_PATH}/include)
set(POCKETPY_LIBRARY     ${POCKETPY_PATH}_build/pocketpy.lib)

ExternalProject_Add(pocketpy_devel
  URL https://github.com/pocketpy/pocketpy/archive/refs/tags/${POCKETPY_DEVEL_PKG}
  URL_HASH SHA1=30e2dcc881b7c2d657dae09f03e4c852a352c2bf
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${POCKETPY_PATH}/
  BINARY_DIR ${POCKETPY_PATH}_build/
  BUILD_BYPRODUCTS ${POCKETPY_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DPK_ENABLE_OS=ON
    -DPK_BUILD_MODULE_LZ4=OFF
  INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${POCKETPY_PATH}_build/pocketpy.dll ${CMAKE_CURRENT_SOURCE_DIR}/export)

# inih
set(INIH_VERSION     "58")
set(INIH_DEVEL_PKG   r${INIH_VERSION}.zip)
set(INIH_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/inih-${INIH_VERSION})
set(INIH_INCLUDE_DIR ${INIH_PATH})
set(INIH_LIBRARY     ${INIH_PATH}_build/inih.lib)

ExternalProject_Add(inih_devel
  URL https://github.com/benhoyt/inih/archive/refs/tags/${INIH_DEVEL_PKG}
  URL_HASH SHA1=4ab39673da3a84ccf9828428616acced69f0528e
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${INIH_PATH}/
  BINARY_DIR ${INIH_PATH}_build/
  BUILD_BYPRODUCTS ${INIH_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_inih.cmake" ${INIH_PATH}/CMakeLists.txt)

# Lua
set(LUA_VERSION     "5.4.7")
set(LUA_DEVEL_PKG   lua-${LUA_VERSION}.tar.gz)
set(LUA_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/lua-${LUA_VERSION}_${PLATFORM})
set(LUA_INCLUDE_DIR ${LUA_PATH}/src)
set(LUA_LIBRARY     ${LUA_PATH}/lua.lib)

ExternalProject_Add(Lua_devel
    URL https://www.lua.org/ftp/${LUA_DEVEL_PKG}
    URL_HASH SHA1=29b54f97dab8631f52ee21a44871622eaefbe235
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${LUA_PATH}/
    BUILD_BYPRODUCTS ${LUA_LIBRARY}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_lua.cmake" ${LUA_PATH}/CMakeLists.txt)

# SDL3
set(SDL3_VERSION "3.2.16")

set(SDL3_DEVEL_PKG SDL3-devel-${SDL3_VERSION}-VC.zip)
set(SDL3_PLATFORM  "x64")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SDL3_PLATFORM "x86")
endif()

set(SDL3_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/SDL3-${SDL3_VERSION}_${SDL3_PLATFORM})

ExternalProject_Add(SDL3_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/${SDL3_DEVEL_PKG}
    URL_HASH SHA1=7f060c12b0f0a19ab82621680ebe0e84a64ddecc
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL3_PATH}/
    BUILD_BYPRODUCTS ${SDL3_PATH}/lib/${SDL3_PLATFORM}/SDL3.lib

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build step."

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
        ${SDL3_PATH}/lib/${SDL3_PLATFORM}/SDL3.dll ${CMAKE_CURRENT_SOURCE_DIR}/export

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_sdl3.cmake" ${SDL3_PATH}/CMakeLists.txt)

set(SDL3_INCLUDE_DIR ${SDL3_PATH}/include)
set(SDL3_LIBRARY     ${SDL3_PATH}/lib/${SDL3_PLATFORM}/SDL3.lib)

# PCAN-Basic API
set(PCAN_VERSION_WINDOWS "4.10.1.968")

set(PCAN_PLATFORM  "x64")
set(PCAN_PATH      "${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/PCAN-Basic_API_Windows")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PCAN_PLATFORM "Win32")
endif()

set(PCAN_DEVEL_PKG  PCAN-Basic_Windows-${PCAN_VERSION_WINDOWS}.zip)
set(PCAN_DEVEL_URL  https://canopenterm.de/mirror)
set(PCAN_DEVEL_HASH 66c7941f1bb93294621437f3d8774845ab0b9659)

ExternalProject_Add(PCAN_devel
    URL ${PCAN_DEVEL_URL}/${PCAN_DEVEL_PKG}
    URL_HASH SHA1=${PCAN_DEVEL_HASH}
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${PCAN_PATH}/
    BUILD_BYPRODUCTS ${PCAN_PATH}/${PCAN_PLATFORM}/VC_LIB/PCANBasic.lib

    BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Skipping build step."

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${PCAN_PATH}/${PCAN_PLATFORM}/PCANBasic.dll ${CMAKE_CURRENT_SOURCE_DIR}/export

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_pcan.cmake" ${PCAN_PATH}/CMakeLists.txt)

set(PCAN_INCLUDE_DIR ${PCAN_PATH}/Include)
set(PCAN_LIBRARY     ${PCAN_PATH}/${PCAN_PLATFORM}/VC_LIB/PCANBasic.lib)

# dirent
set(DIRENT_VERSION   "1.24")
set(DIRENT_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/dirent-${DIRENT_VERSION})
set(DIRENT_DEVEL_PKG ${DIRENT_VERSION}.zip)

ExternalProject_Add(dirent_devel
    URL https://github.com/tronkko/dirent/archive/refs/tags/${DIRENT_DEVEL_PKG}
    URL_HASH SHA1=70b02369071572dd1b080057a6b9170dec04868d
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${DIRENT_PATH}/

    BUILD_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping build step."
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_dirent.cmake" ${DIRENT_PATH}/CMakeLists.txt)

set(DIRENT_INCLUDE_DIR ${DIRENT_PATH}/include)

add_dependencies(${PROJECT_NAME} PCAN_devel)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(ADDITIONAL_RUNTIME
        ucrt
        legacy_stdio_definitions
        legacy_stdio_wide_specifiers)
endif()

set(PLATFORM_LIBS
    ${CJSON_LIBRARY}
    ${PCAN_LIBRARY}
    ${POCKETPY_LIBRARY}
    ${SDL3_LIBRARY}
    ${INIH_LIBRARY}
    ${LUA_LIBRARY}
    ${ADDITIONAL_RUNTIME})

set(PLATFORM_CORE_DEPS
    inih_devel
    Lua_devel
    PCAN_devel
    pocketpy_devel
    SDL3_devel)

include_directories(
    SYSTEM ${SDL3_INCLUDE_DIR}
    SYSTEM ${PCAN_INCLUDE_DIR}
    SYSTEM ${PCAN_INCLUDE_DIR}/../src/pcan/driver
    SYSTEM ${PCAN_INCLUDE_DIR}/../src/pcan/lib
    SYSTEM ${POCKETPY_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
    SYSTEM ${INIH_INCLUDE_DIR}
    SYSTEM ${CJSON_INCLUDE_DIR}
    SYSTEM ${DIRENT_INCLUDE_DIR})

add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    WIN32)
