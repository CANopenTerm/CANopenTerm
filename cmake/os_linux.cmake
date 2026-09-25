cmake_minimum_required(VERSION 3.16)

# Include centralized dependency configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

if(BUILD_TESTS)
  # CMocka
  dep_cmocka()
  set(CMocka_DEVEL_PKG   cmocka-${DEP_CMOCKA_VERSION}.tar.xz)
  set(CMocka_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${DEP_CMOCKA_VERSION})
  set(CMocka_BUILD_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/cmocka-${DEP_CMOCKA_VERSION}_build)
  set(CMocka_INCLUDE_DIR ${CMocka_PATH}/include)
  set(CMocka_LIBRARY     ${CMocka_BUILD_PATH}/src/libcmocka.a)

  ExternalProject_Add(CMocka_devel
    URL ${DEP_CMOCKA_URL_BASE}/${CMocka_DEVEL_PKG}
    URL_HASH SHA1=${DEP_CMOCKA_SHA1}
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
    DOWNLOAD_NO_PROGRESS true
    DOWNLOAD_EXTRACT_TIMESTAMP true
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

    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
  )
endif(BUILD_TESTS)

# Use system-provided libraries instead of fetching them at build time.
if (USE_SYSTEM_LIBS)
  find_package(SDL3 REQUIRED)
  if (SDL3_FOUND)
    include_directories(${SDL3_INCLUDE_DIRS})
    set(PLATFORM_LIBS ${PLATFORM_LIBS} ${SDL3_LIBRARIES})
  else()
    message(FATAL_ERROR "SDL3 not found")
  endif()

  find_package(CANvenient REQUIRED)
  if (CANvenient_FOUND)
    include_directories(${CANvenient_INCLUDE_DIR})
    set(PLATFORM_LIBS ${PLATFORM_LIBS} ${CANvenient_LIBRARIES})
  else()
    message(FATAL_ERROR "CANvenient not found")
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
    add_compile_definitions(LUA_USE_LINUX)
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

  find_package(isocline REQUIRED)
  if (isocline_FOUND)
    include_directories(${isocline_INCLUDE_DIRS})
    set(PLATFORM_LIBS ${PLATFORM_LIBS} ${isocline_LIBRARIES})
  else()
    message(FATAL_ERROR "isocline not found")
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
    history
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/codb/
    DESTINATION /usr/share/CANopenTerm/codb
    FILES_MATCHING PATTERN "*"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/dbc/
    DESTINATION /usr/share/CANopenTerm/dbc
    FILES_MATCHING PATTERN "*"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/eds/
    DESTINATION /usr/share/CANopenTerm/eds
    FILES_MATCHING PATTERN "*"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/
    DESTINATION /usr/share/CANopenTerm/scripts
    FILES_MATCHING PATTERN "*.lua" PATTERN "*.py"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/core/
    DESTINATION /usr/share/CANopenTerm/scripts/core
    FILES_MATCHING PATTERN "*"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/isobus/
    DESTINATION /usr/share/CANopenTerm/scripts/isobus
    FILES_MATCHING PATTERN "*"
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/export/scripts/obd2/
    DESTINATION /usr/share/CANopenTerm/scripts/obd2
    FILES_MATCHING PATTERN "*"
  )

  install(
    PROGRAMS $<TARGET_FILE:CANopenTerm>
    DESTINATION /usr/bin
  )

  install(
    PROGRAMS $<TARGET_FILE:codb2json>
    DESTINATION /usr/bin
  )

  return()
endif()

# cJSON
dep_cjson()
set(CJSON_DEVEL_PKG   "v${DEP_CJSON_VERSION}.tar.gz")
set(CJSON_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/CJSON-${DEP_CJSON_VERSION})
set(CJSON_INCLUDE_DIR ${CJSON_PATH})
set(CJSON_LIBRARY     ${CJSON_PATH}_build/libcjson.so.${DEP_CJSON_VERSION})

ExternalProject_Add(cJSON_devel
  URL ${DEP_CJSON_URL_BASE}/${CJSON_DEVEL_PKG}
  URL_HASH SHA1=${DEP_CJSON_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${CJSON_PATH}/
  BINARY_DIR ${CJSON_PATH}_build/
  BUILD_BYPRODUCTS ${CJSON_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E echo "Skipping install step."
)

# CANvenient
dep_canvenient()
set(CANVENIENT_DEVEL_PKG   "v${DEP_CANVENIENT_VERSION}.tar.gz")
set(CANVENIENT_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/CANvenient-${DEP_CANVENIENT_VERSION})
set(CANVENIENT_INCLUDE_DIR ${CANVENIENT_PATH}/include)
set(CANVENIENT_LIBRARY     ${CANVENIENT_PATH}_build/libCANvenient.so)

ExternalProject_Add(CANvenient_devel
  URL ${DEP_CANVENIENT_URL_BASE_LINUX}/${CANVENIENT_DEVEL_PKG}
  URL_HASH SHA1=${DEP_CANVENIENT_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${CANVENIENT_PATH}/
  BINARY_DIR ${CANVENIENT_PATH}_build/
  BUILD_BYPRODUCTS ${CANVENIENT_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${CANVENIENT_PATH}_build/libCANvenient.so
    ${CMAKE_CURRENT_SOURCE_DIR}/export
  )

# pocketpy
dep_pocketpy()
set(POCKETPY_DEVEL_PKG   "v${DEP_POCKETPY_VERSION}.tar.gz")
set(POCKETPY_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/pocketpy-${DEP_POCKETPY_VERSION})
set(POCKETPY_INCLUDE_DIR ${POCKETPY_PATH}/include)
set(POCKETPY_LIBRARY     ${POCKETPY_PATH}_build/libpocketpy.a)

ExternalProject_Add(pocketpy_devel
  URL ${DEP_POCKETPY_URL_BASE}/${POCKETPY_DEVEL_PKG}
  URL_HASH SHA1=${DEP_POCKETPY_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
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
  PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step."
)

# inih
dep_inih()
set(INIH_DEVEL_PKG   r${DEP_INIH_VERSION}.zip)
set(INIH_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/inih-${DEP_INIH_VERSION})
set(INIH_INCLUDE_DIR ${INIH_PATH})
set(INIH_LIBRARY     ${INIH_PATH}_build/libinih.a)

ExternalProject_Add(inih_devel
  URL ${DEP_INIH_URL_BASE}/${INIH_DEVEL_PKG}
  URL_HASH SHA1=${DEP_INIH_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${INIH_PATH}/
  BINARY_DIR ${INIH_PATH}_build/
  BUILD_BYPRODUCTS ${INIH_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_inih.cmake" ${INIH_PATH}/CMakeLists.txt
)

# isocline
dep_isocline()
set(ISOCLINE_DEVEL_PKG   "v${DEP_ISOCLINE_VERSION}.tar.gz")
set(ISOCLINE_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/isocline-${DEP_ISOCLINE_VERSION})
set(ISOCLINE_INCLUDE_DIR ${ISOCLINE_PATH}/include)
set(ISOCLINE_LIBRARY     ${ISOCLINE_PATH}_build/libisocline.a)

ExternalProject_Add(isocline_devel
  URL ${DEP_ISOCLINE_URL_BASE}/${ISOCLINE_DEVEL_PKG}
  URL_HASH SHA1=${DEP_ISOCLINE_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${ISOCLINE_PATH}/
  BINARY_DIR ${ISOCLINE_PATH}_build/
  BUILD_BYPRODUCTS ${ISOCLINE_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
  )

# Lua
dep_lua()
set(LUA_DEVEL_PKG   lua-${DEP_LUA_VERSION}.tar.gz)
set(LUA_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/lua-${DEP_LUA_VERSION}_${PLATFORM})
set(LUA_INCLUDE_DIR ${LUA_PATH}/src)
set(LUA_LIBRARY     ${LUA_PATH}/liblua.a)

ExternalProject_Add(Lua_devel
  URL ${DEP_LUA_URL_BASE}/${LUA_DEVEL_PKG}
  URL_HASH SHA1=${DEP_LUA_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${LUA_PATH}/
  BUILD_BYPRODUCTS ${LUA_LIBRARY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_lua.cmake" ${LUA_PATH}/CMakeLists.txt
)

# SDL3
dep_sdl3()
set(SDL3_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/SDL3-${DEP_SDL3_VERSION})
set(SDL3_DEVEL_PKG SDL3-${DEP_SDL3_VERSION}.tar.gz)

ExternalProject_Add(SDL3_devel
  URL ${DEP_SDL3_URL_BASE}/${SDL3_DEVEL_PKG}
  URL_HASH SHA1=${DEP_SDL3_SHA1_LINUX}
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  DOWNLOAD_EXTRACT_TIMESTAMP true
  TLS_VERIFY true
  SOURCE_DIR ${SDL3_PATH}/
  BINARY_DIR ${SDL3_PATH}_build
  BUILD_BYPRODUCTS ${SDL3_PATH}_build/libSDL3.so
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSDL_UNIX_CONSOLE_BUILD=ON
    -DSDL_X11_XTEST=OFF

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."
  PATCH_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping patch step."
)

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
      /opt/vc/lib/libvcos.so
    )

    include_directories(/opt/vc/include)
    include_directories(/opt/vc/include/interface/vcos/pthreads)
    include_directories(/opt/vc/include/interface/vmcs_host/linux)
  endif()
endif()

set(PLATFORM_LIBS
  ${CANVENIENT_LIBRARY}
  ${CJSON_LIBRARY}
  ${SDL3_LIBRARY}
  ${INIH_LIBRARY}
  ${ISOCLINE_LIBRARY}
  ${LUA_LIBRARY}
  ${POCKETPY_LIBRARY}
  dl
  m
  pthread
  readline
  history
)

set(PLATFORM_CORE_DEPS
  CANvenient_devel
  cJSON_devel
  inih_devel
  isocline_devel
  Lua_devel
  pocketpy_devel
  SDL3_devel
)

include_directories(
  SYSTEM ${SDL3_INCLUDE_DIR}
  SYSTEM ${CANVENIENT_INCLUDE_DIR}
  SYSTEM ${LUA_INCLUDE_DIR}
  SYSTEM ${POCKETPY_INCLUDE_DIR}
  SYSTEM ${INIH_INCLUDE_DIR}
  SYSTEM ${ISOCLINE_INCLUDE_DIR}
  SYSTEM ${CJSON_INCLUDE_DIR}
)
