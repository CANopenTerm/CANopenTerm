# This file defines all external dependencies used by CANopenTerm.
# It serves as a single source of truth for versions and checksums.
# Platform-specific details (URLs, library formats) are handled in os_linux.cmake and os_windows.cmake

cmake_minimum_required(VERSION 3.16)

# CMocka
function(dep_cmocka)
  set(DEP_CMOCKA_VERSION "1.1.7" PARENT_SCOPE)
  set(DEP_CMOCKA_URL_BASE "https://cmocka.org/files/1.1" PARENT_SCOPE)
  set(DEP_CMOCKA_PACKAGE_NAME "cmocka-1.1.7.tar.xz" PARENT_SCOPE)
  set(DEP_CMOCKA_SHA1 "04cf44545a22e7182803a092a30af5c1a42c31bc" PARENT_SCOPE)
endfunction()

# cJSON
function(dep_cjson)
  set(DEP_CJSON_VERSION "1.7.19" PARENT_SCOPE)
  set(DEP_CJSON_URL_BASE "https://github.com/DaveGamble/cJSON/archive/refs/tags" PARENT_SCOPE)
  set(DEP_CJSON_SHA1_LINUX "e66ddd2f99fd321ab53a694e6c74698eb987d056" PARENT_SCOPE)
  set(DEP_CJSON_SHA1_WINDOWS "affe45d703ca611ca25af129b3ae51621760c9dd" PARENT_SCOPE)
endfunction()

# CANvenient
function(dep_canvenient)
  set(DEP_CANVENIENT_VERSION "1.02" PARENT_SCOPE)
  set(DEP_CANVENIENT_URL_BASE_LINUX "https://github.com/CANopenTerm/CANvenient/archive/refs/tags" PARENT_SCOPE)
  set(DEP_CANVENIENT_URL_BASE_WINDOWS "https://github.com/CANopenTerm/CANvenient/releases/download/v1.02" PARENT_SCOPE)
  set(DEP_CANVENIENT_SHA1_LINUX "9b850bde87edf0bd5202d7cd0a8bd6dd0474a7aa" PARENT_SCOPE)
  set(DEP_CANVENIENT_SHA1_WINDOWS "61f9715d223e353151ba5f6d5e6b95f7d0387e26" PARENT_SCOPE)
endfunction()

# pocketpy
function(dep_pocketpy)
  set(DEP_POCKETPY_VERSION "2.2.0" PARENT_SCOPE)
  set(DEP_POCKETPY_URL_BASE "https://github.com/pocketpy/pocketpy/archive/refs/tags" PARENT_SCOPE)
  set(DEP_POCKETPY_SHA1_LINUX "68dabcf74ef6b6a23bdefb3b79001d1cc6da49c5" PARENT_SCOPE)
  set(DEP_POCKETPY_SHA1_WINDOWS "9b9bec8c44829f8d0c68aa782d7b5b847a98cdc6" PARENT_SCOPE)
endfunction()

# inih
function(dep_inih)
  set(DEP_INIH_VERSION "62" PARENT_SCOPE)
  set(DEP_INIH_URL_BASE "https://github.com/benhoyt/inih/archive/refs/tags" PARENT_SCOPE)
  set(DEP_INIH_SHA1_LINUX "5340713ce7695e01c848cc4f5366c7eb79e59191" PARENT_SCOPE)
  set(DEP_INIH_SHA1_WINDOWS "5340713ce7695e01c848cc4f5366c7eb79e59191" PARENT_SCOPE)
endfunction()

# isocline
function(dep_isocline)
  set(DEP_ISOCLINE_VERSION "1.1.0" PARENT_SCOPE)
  set(DEP_ISOCLINE_URL_BASE "https://github.com/daanx/isocline/archive/refs/tags" PARENT_SCOPE)
  set(DEP_ISOCLINE_SHA1_LINUX "3f0ca7cd0cfcb1f0e9ec0b6fbda4666d8da99bd8" PARENT_SCOPE)
  set(DEP_ISOCLINE_SHA1_WINDOWS "1baa0220ed682920ef30520fb10caa5f642615d8" PARENT_SCOPE)
endfunction()

# Lua
function(dep_lua)
  set(DEP_LUA_VERSION "5.5.1" PARENT_SCOPE)
  set(DEP_LUA_URL_BASE "https://www.lua.org/ftp" PARENT_SCOPE)
  set(DEP_LUA_SHA1_LINUX "b32be1c883e1c1964a0d2cafb22806589ceb716f" PARENT_SCOPE)
  set(DEP_LUA_SHA1_WINDOWS "b32be1c883e1c1964a0d2cafb22806589ceb716f" PARENT_SCOPE)
endfunction()

# SDL3
function(dep_sdl3)
  set(DEP_SDL3_VERSION "3.4.16" PARENT_SCOPE)
  # Linux and generic URL base
  set(DEP_SDL3_URL_BASE "https://github.com/libsdl-org/SDL/releases/download/release-3.4.16" PARENT_SCOPE)
  set(DEP_SDL3_SHA1_LINUX "f4b6a2f11208bd02b73ac6f1b2bc03409b23fd63" PARENT_SCOPE)
  set(DEP_SDL3_SHA1_WINDOWS "edcf1f567837e7464cf80df402661b41acd78dbf" PARENT_SCOPE)
endfunction()

# dirent
function(dep_dirent)
  set(DEP_DIRENT_VERSION "1.26" PARENT_SCOPE)
  set(DEP_DIRENT_URL_BASE "https://github.com/tronkko/dirent/archive/refs/tags" PARENT_SCOPE)
  # Windows: 1.26.zip (SHA1=5a9dfd1204dcf29d418ec1678fd961a42b84f5fb)
  set(DEP_DIRENT_SHA1_WINDOWS "5a9dfd1204dcf29d418ec1678fd961a42b84f5fb" PARENT_SCOPE)
endfunction()

# Convenience function to get all dependency versions at once.
function(get_all_dependency_versions)
  dep_cmocka()
  dep_cjson()
  dep_canvenient()
  dep_pocketpy()
  dep_inih()
  dep_isocline()
  dep_lua()
  dep_sdl3()
  dep_dirent()

  message(STATUS "Dependency Versions:")
  message(STATUS "  CMocka: ${DEP_CMOCKA_VERSION}")
  message(STATUS "  cJSON: ${DEP_CJSON_VERSION}")
  message(STATUS "  CANvenient: ${DEP_CANVENIENT_VERSION}")
  message(STATUS "  pocketpy: ${DEP_POCKETPY_VERSION}")
  message(STATUS "  inih: ${DEP_INIH_VERSION}")
  message(STATUS "  isocline: ${DEP_ISOCLINE_VERSION}")
  message(STATUS "  Lua: ${DEP_LUA_VERSION}")
  message(STATUS "  SDL3: ${DEP_SDL3_VERSION}")
  message(STATUS "  dirent: ${DEP_DIRENT_VERSION}")
endfunction()
