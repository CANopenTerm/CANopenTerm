# CMocka
set(CMocka_VERSION     "1.1.7")
set(CMocka_DEVEL_PKG   cmocka-${CMocka_VERSION}.tar.xz)
set(CMocka_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/deps/cmocka-${CMocka_VERSION})
set(CMocka_BUILD_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/deps/cmocka-${CMocka_VERSION}_build)
set(CMocka_INCLUDE_DIR ${CMocka_PATH}/include)

if(UNIX)
  set(CMocka_LIBRARY ${CMocka_BUILD_PATH}/src/libcmocka.a)  
endif(UNIX)

if(WIN32)
  set(CMocka_LIBRARY ${CMocka_BUILD_PATH}/src/cmocka.lib)  
endif(WIN32)

ExternalProject_Add(CMocka_devel
  URL https://cmocka.org/files/1.1/${CMocka_DEVEL_PKG}
  URL_HASH SHA1=04cf44545a22e7182803a092a30af5c1a42c31bc
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps
  DOWNLOAD_NO_PROGRESS true
  TLS_VERIFY true
  SOURCE_DIR ${CMocka_PATH}
  BINARY_DIR ${CMocka_BUILD_PATH}
  BUILD_BYPRODUCTS ${CMocka_LIBRARY}

  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON
    -DBUILD_SHARED_LIBS=OFF
    -DWITH_EXAMPLES=OFF 

  INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Skipping install step.")

add_executable(
  run_unit_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/run_unit_tests.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_dict.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_nmt_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test_wrapper.c)

add_dependencies(
  run_unit_tests
  core
  CMocka_devel
  SDL2_devel
  Lua_devel)

target_link_libraries(
  run_unit_tests
  core
  ${CMocka_LIBRARY}
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  ${LUA_LIBRARY}
  ${PLATFORM_LIBS})

target_link_options(
  run_unit_tests
  PUBLIC
  -Wl,--wrap=can_write)

include_directories(
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
  SYSTEM ${CMocka_INCLUDE_DIR}
  SYSTEM ${SDL2_INCLUDE_DIR}
  SYSTEM ${LUA_INCLUDE_DIR})
