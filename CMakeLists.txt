cmake_minimum_required(VERSION 3.0)
project(awcotn)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -rdynamic -ggdb -O0 -Wall  -Werror \
-Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/apps/awcotn/include)
link_directories(/apps/awcotn/lib)

find_library(YAMLCPP yaml-cpp)

set(LIB_SRC
    awcotn/log.cc
    awcotn/util.cc
    awcotn/config.cc
    )

add_library(awcotn SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(awcotn) #__FILE__
#add_library(awcotn_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(awcotn_static PROPERTIES OUTPUT_NAME "awcotn")

add_executable(test tests/test.cc)
force_redefine_file_macro_for_sources(test) #__FILE__
add_dependencies(test awcotn)
target_link_libraries(test awcotn ${YAMLCPP})

add_executable(test_config tests/test_config.cc)
force_redefine_file_macro_for_sources(test_config) #__FILE__
add_dependencies(test_config awcotn)
target_link_libraries(test_config awcotn yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
