cmake_minimum_required(VERSION 3.0)
project(awcotn)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -rdynamic -ggdb -O0 -Wall  -Werror -Wno-unused-function -Wno-deprecated")

include_directories(${PROJECT_SOURCE_DIR})

set(LIB_SRC
    awcotn/log.cc
    awcotn/util.cc
    )

add_library(awcotn SHARED ${LIB_SRC})
#add_library(awcotn_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(awcotn_static PROPERTIES OUTPUT_NAME "awcotn")

add_executable(test tests/test.cc)
add_dependencies(test awcotn)
target_link_libraries(test awcotn)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
