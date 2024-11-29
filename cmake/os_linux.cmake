# Use system dependencies when building in a Yocto environment.
if (BUILD_YOCTO)
    find_package(SDL2 REQUIRED)
    if (SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${SDL2_LIBRARIES})
    else()
        message(FATAL_ERROR "SDL2 not found")
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

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/dbc/
            DESTINATION /usr/share/CANopenTerm/dbc
            FILES_MATCHING PATTERN "*")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/eds/
            DESTINATION /usr/share/CANopenTerm/eds
            FILES_MATCHING PATTERN "*")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/
            DESTINATION /usr/share/CANopenTerm/scripts
            FILES_MATCHING PATTERN "*.lua" PATTERN "*.py")

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/lua/
            DESTINATION /usr/share/CANopenTerm/scripts/lua
            FILES_MATCHING PATTERN "*")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CANopenTerm
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
set(POCKETPY_VERSION     "2.0.1")
set(POCKETPY_DEVEL_PKG   "v${POCKETPY_VERSION}.tar.gz")
set(POCKETPY_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/pocketpy-${POCKETPY_VERSION})
set(POCKETPY_INCLUDE_DIR ${POCKETPY_PATH}/include)
set(POCKETPY_LIBRARY     ${POCKETPY_PATH}_build/libpocketpy.so)

ExternalProject_Add(pocketpy_devel
  URL https://github.com/pocketpy/pocketpy/archive/refs/tags/${POCKETPY_DEVEL_PKG}
  URL_HASH SHA1=e4991b6f76c48a07ac51d34360f393eacead4e91
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${POCKETPY_PATH}/
  BINARY_DIR ${POCKETPY_PATH}_build/
  BUILD_BYPRODUCTS ${POCKETPY_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

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
set(LUA_DEVEL_PKG   v${LUA_VERSION}.zip)
set(LUA_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/lua-${LUA_VERSION}_${PLATFORM})
set(LUA_INCLUDE_DIR ${LUA_PATH})
set(LUA_LIBRARY     ${LUA_PATH}/liblua.a)

ExternalProject_Add(Lua_devel
  URL https://github.com/lua/lua/archive/refs/tags/${LUA_DEVEL_PKG}
  URL_HASH SHA1=1c8e1ff7988e3eb7326b495a83875ea931881090
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

# SDL2
set(SDL2_VERSION  "2.30.9")
set(SDL2_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/SDL2-${SDL2_VERSION})
set(SDL2_DEVEL_PKG SDL2-${SDL2_VERSION}.tar.gz)

ExternalProject_Add(SDL2_devel
  URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
  URL_HASH SHA1=9403df0573d47f62f2de074b582b87576bb4abbc
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${SDL2_PATH}/
  BINARY_DIR ${SDL2_PATH}_build
  BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2.a
  BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2main.a
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
  PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step.")

set(SDL2_INCLUDE_DIR ${SDL2_PATH}/include)
set(SDL2_LIBRARY     ${SDL2_PATH}_build/libSDL2.a)
set(SDL2MAIN_LIBRARY ${SDL2_PATH}_build/libSDL2main.a)

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
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  ${INIH_LIBRARY}
  ${LUA_LIBRARY}
  ${POCKETPY_LIBRARY}
  dl
  m
  pthread
  readline
  history)

set(PLATFORM_CORE_DEPS
  inih_devel
  Lua_devel
  pocketpy_devel
  SDL2_devel)

include_directories(
  SYSTEM ${SDL2_INCLUDE_DIR}
  SYSTEM ${LUA_INCLUDE_DIR}
  SYSTEM ${POCKETPY_INCLUDE_DIR}
  SYSTEM ${INIH_INCLUDE_DIR}
  SYSTEM ${CJSON_INCLUDE_DIR})
