if(BUILD_TESTS)
    # CMocka
    set(CMocka_VERSION     "1.1.7")
    set(CMocka_DEVEL_PKG   cmocka-${CMocka_VERSION}.tar.xz)
    set(CMocka_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${CMocka_VERSION})
    set(CMocka_BUILD_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${CMocka_VERSION}_build)
    set(CMocka_INCLUDE_DIR ${CMocka_PATH}/include)
    set(CMocka_LIBRARY     ${CMocka_BUILD_PATH}/src/libcmocka.a)

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
            -DBUILD_SHARED_LIBS=OFF
            -DWITH_EXAMPLES=OFF

        PATCH_COMMAND ${CMAKE_COMMAND} -DCMocka_PATH=${CMocka_PATH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_cmocka.cmake

        INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step.")
endif(BUILD_TESTS)

# Use system dependencies when building in a Yocto environment.
if (BUILD_YOCTO)
    find_package(SDL3 REQUIRED)
    if (SDL3_FOUND)
        include_directories(${SDL3_INCLUDE_DIRS})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${SDL3_LIBRARIES})
    else()
        message(FATAL_ERROR "SDL3 not found")
    endif()

    find_package(cJSON REQUIRED)
    if (cJSON_FOUND)
        include_directories(${cJSON_INCLUDE_DIR})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${cJSON_LIBRARIES})
    else()
        message(FATAL_ERROR "cJSON not found")
    endif()

    find_package(Lua REQUIRED)
    if (LUA_FOUND)
        include_directories(${LUA_INCLUDE_DIR})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${LUA_LIBRARIES})
        add_compile_definitions(LUA_USE_C89 LUA_USE_LINUX)
    else()
        message(FATAL_ERROR "Lua not found")
    endif()

    find_package(inih REQUIRED)
    if (inih_FOUND)
        include_directories(${inih_INCLUDE_DIRS})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${inih_LIBRARIES})
    else()
        message(FATAL_ERROR "inih not found")
    endif()

    find_package(pocketpy REQUIRED)
    if (pocketpy_FOUND)
        include_directories(${pocketpy_INCLUDE_DIRS})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${pocketpy_LIBRARIES})
    else()
        message(FATAL_ERROR "pocketpy not found")
    endif()

    set(PLATFORM_LIBS
        ${PLATFORM_LIBS}
        dl
        m
        pthread
        readline
        history)

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/codb/
            DESTINATION /usr/share/CANopenTerm/codb
            FILES_MATCHING PATTERN "*")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/dbc/
            DESTINATION /usr/share/CANopenTerm/dbc
            FILES_MATCHING PATTERN "*")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/eds/
            DESTINATION /usr/share/CANopenTerm/eds
            FILES_MATCHING PATTERN "*")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/
            DESTINATION /usr/share/CANopenTerm/scripts
            FILES_MATCHING PATTERN "*.lua" PATTERN "*.py")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/utils/
            DESTINATION /usr/share/CANopenTerm/scripts/utils
            FILES_MATCHING PATTERN "*")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CANopenTerm
            DESTINATION /usr/bin)

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/codb2json
            DESTINATION /usr/bin)

    return()
endif()

# cJSON
set(CJSON_VERSION     "1.7.18")
set(CJSON_DEVEL_PKG   "v${CJSON_VERSION}.tar.gz")
set(CJSON_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/CJSON-${CJSON_VERSION})
set(CJSON_INCLUDE_DIR ${CJSON_PATH})
set(CJSON_LIBRARY     ${CJSON_PATH}_build/libcjson.so.${CJSON_VERSION})

ExternalProject_Add(cJSON_devel
    URL https://github.com/DaveGamble/cJSON/archive/refs/tags/${CJSON_DEVEL_PKG}
    URL_HASH SHA1=3e3408c124a2c885e2724ff88d7f5473cda53038
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${CJSON_PATH}/
    BINARY_DIR ${CJSON_PATH}_build/
    BUILD_BYPRODUCTS ${CJSON_LIBRARY}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E echo "Skipping install step.")

# pocketpy
set(POCKETPY_VERSION     "2.0.6")
set(POCKETPY_DEVEL_PKG   "v${POCKETPY_VERSION}.tar.gz")
set(POCKETPY_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/pocketpy-${POCKETPY_VERSION})
set(POCKETPY_INCLUDE_DIR ${POCKETPY_PATH}/include)
set(POCKETPY_LIBRARY     ${POCKETPY_PATH}_build/libpocketpy.a)

ExternalProject_Add(pocketpy_devel
    URL https://github.com/pocketpy/pocketpy/archive/refs/tags/${POCKETPY_DEVEL_PKG}
    URL_HASH SHA1=723a8cd3926b91e60587c0efdd963ba875d79e3d
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${POCKETPY_PATH}/
    BINARY_DIR ${POCKETPY_PATH}_build/
    BUILD_BYPRODUCTS ${POCKETPY_LIBRARY}
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPK_BUILD_STATIC_LIB=ON
        -DPK_ENABLE_OS=ON
        -DPK_BUILD_MODULE_LZ4=OFF
        -DPK_BUILD_WITH_IPO=OFF
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
    PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step.")

# inih
set(INIH_VERSION     "58")
set(INIH_DEVEL_PKG   r${INIH_VERSION}.zip)
set(INIH_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/inih-${INIH_VERSION})
set(INIH_INCLUDE_DIR ${INIH_PATH})
set(INIH_LIBRARY     ${INIH_PATH}_build/libinih.a)

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
set(LUA_LIBRARY     ${LUA_PATH}/liblua.a)

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
set(SDL3_VERSION  "3.2.8")
set(SDL3_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/SDL3-${SDL3_VERSION})
set(SDL3_DEVEL_PKG SDL3-${SDL3_VERSION}.tar.gz)

ExternalProject_Add(SDL3_devel
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/${SDL3_DEVEL_PKG}
    URL_HASH SHA1=e5c1749aa6bd848f74e07ee3559442acdc06991a
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    TLS_VERIFY true
    SOURCE_DIR ${SDL3_PATH}/
    BINARY_DIR ${SDL3_PATH}_build
    BUILD_BYPRODUCTS ${SDL3_PATH}_build/libSDL3.so
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DSDL_UNIX_CONSOLE_BUILD=ON

    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
    PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step.")

set(SDL3_INCLUDE_DIR ${SDL3_PATH}/include)
set(SDL3_LIBRARY     ${SDL3_PATH}_build/libSDL3.so)

if(EXISTS "/proc/cpuinfo")
    file(READ "/proc/cpuinfo" CPUINFO)
    if(CPUINFO MATCHES "Raspberry Pi")
        message(STATUS "Raspberry Pi detected")

        link_directories(/opt/vc/lib)

        target_link_libraries(${PROJECT_NAME}
            /opt/vc/lib/libbcm_host.so
            /opt/vc/lib/libvchiq_arm.so
            /opt/vc/lib/libvcos.so)

        include_directories(/opt/vc/include)
        include_directories(/opt/vc/include/interface/vcos/pthreads)
        include_directories(/opt/vc/include/interface/vmcs_host/linux)
    endif()
endif()

set(PLATFORM_LIBS
    ${CJSON_LIBRARY}
    ${SDL3_LIBRARY}
    ${INIH_LIBRARY}
    ${LUA_LIBRARY}
    ${POCKETPY_LIBRARY}
    dl
    m
    pthread
    readline
    history)

set(PLATFORM_CORE_DEPS
    cJSON_devel
    inih_devel
    Lua_devel
    pocketpy_devel
    SDL3_devel)

include_directories(
    SYSTEM ${SDL3_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
    SYSTEM ${POCKETPY_INCLUDE_DIR}
    SYSTEM ${INIH_INCLUDE_DIR}
    SYSTEM ${CJSON_INCLUDE_DIR})
