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

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_inih.cmake" ${INIH_PATH}/CMakeLists.txt)

# Lua
set(LUA_VERSION     "5.4.6")
set(LUA_DEVEL_PKG   v${LUA_VERSION}.zip)
set(LUA_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/lua-${LUA_VERSION}_${PLATFORM})
set(LUA_INCLUDE_DIR ${LUA_PATH})
set(LUA_LIBRARY     ${LUA_PATH}/liblua.a)

ExternalProject_Add(Lua_devel
  URL https://github.com/lua/lua/archive/refs/tags/${LUA_DEVEL_PKG}
  URL_HASH SHA1=96abb80f46e2c6548b47632384205bddfaeb6c37
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${LUA_PATH}/
  BUILD_BYPRODUCTS ${LUA_LIBRARY}

  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step."

  PATCH_COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dep_lua.cmake" ${LUA_PATH}/CMakeLists.txt)

# SDL2
set(SDL2_VERSION  "2.30.5")
set(SDL2_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}/SDL2-${SDL2_VERSION})
set(SDL2_DEVEL_PKG SDL2-${SDL2_VERSION}.tar.gz)

ExternalProject_Add(SDL2_devel
  URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL2_DEVEL_PKG}
  URL_HASH SHA1=4bc5fc5e76f0a1e1a88801ac1323a91c8bfee543
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps_${PLATFORM}
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${SDL2_PATH}/
  BINARY_DIR ${SDL2_PATH}_build
  BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2.a
  BUILD_BYPRODUCTS ${SDL2_PATH}_build/libSDL2main.a

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
  dl
  m
  pthread
  readline
  history)

set(PLATFORM_CORE_DEPS
  inih_devel
  Lua_devel
  SDL2_devel)

include_directories(
  SYSTEM ${SDL2_INCLUDE_DIR}
  SYSTEM ${LUA_INCLUDE_DIR}
  SYSTEM ${INIH_INCLUDE_DIR})
