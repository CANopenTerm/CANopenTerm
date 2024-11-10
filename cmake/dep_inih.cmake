cmake_minimum_required(VERSION 3.10)
project(inih_devel C)

set(inih_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/ini.c)

add_library(inih
  STATIC
  ${inih_sources})

if(WIN32)
  set_target_properties(inih PROPERTIES OUTPUT_NAME "inih" PREFIX "" SUFFIX ".lib")
endif(WIN32)
