cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ISOCLINE_PATH)
  message(FATAL_ERROR "ISOCLINE_PATH is not defined")
endif()

if(NOT DEFINED IS_WINDOWS)
  message(FATAL_ERROR "IS_WINDOWS is not defined")
endif()

file(READ "${ISOCLINE_PATH}/CMakeLists.txt" _contents)

# On Windows, disable the shared library build to avoid "multiple rules generate isocline.lib" error
# Both isocline (STATIC) and isocline_shared (SHARED with OUTPUT_NAME=isocline) try to create isocline.lib
if(IS_WINDOWS)
  # Comment out the entire shared library section
  string(REPLACE 
    "# Shared library (libisocline.so)
# -----------------------------------------------------------------------------

add_library(isocline_shared SHARED \${ic_sources})
set_target_properties(isocline_shared PROPERTIES
    OUTPUT_NAME isocline
    VERSION 1.0.9
    SOVERSION 1
)
target_compile_options(isocline_shared PRIVATE \${ic_cflags})
target_compile_definitions(isocline_shared PRIVATE \${ic_cdefs})
target_include_directories(isocline_shared PUBLIC
    \$<BUILD_INTERFACE:\${CMAKE_CURRENT_SOURCE_DIR}/include>
    \$<INSTALL_INTERFACE:include>
)"
    "# Shared library (libisocline.so) - DISABLED ON WINDOWS
# -------================================================
# add_library(isocline_shared SHARED \${ic_sources})
# set_target_properties(isocline_shared PROPERTIES
#     OUTPUT_NAME isocline
#     VERSION 1.0.9
#     SOVERSION 1
# )
# target_compile_options(isocline_shared PRIVATE \${ic_cflags})
# target_compile_definitions(isocline_shared PRIVATE \${ic_cdefs})
# target_include_directories(isocline_shared PUBLIC
#     \$<BUILD_INTERFACE:\${CMAKE_CURRENT_SOURCE_DIR}/include>
#     \$<INSTALL_INTERFACE:include>
# )"
    _contents "${_contents}")

  # Also update the install rules to not include isocline_shared
  string(REPLACE 
    "install(TARGETS isocline isocline_shared"
    "install(TARGETS isocline"
    _contents "${_contents}")
endif()

file(WRITE "${ISOCLINE_PATH}/CMakeLists.txt" "${_contents}")

