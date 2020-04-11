
cmake_minimum_required(VERSION 3.10)

project(main VERSION 1.0)

set(INCLUDE_DIR 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set(SOURCES
  src/JSON.cpp
  src/main.cpp
)

add_executable(main ${SOURCES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(GNUInstallDirs)

include_directories(
  ${INCLUDE_DIR}
)


# <math.h>
#target_link_libraries(func m)

#target_link_libraries(deep FunCx64)

