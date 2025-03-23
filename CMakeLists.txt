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
    awcotn/thread.cc
    )

add_library(awcotn SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(awcotn) #__FILE__
#add_library(awcotn_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(awcotn_static PROPERTIES OUTPUT_NAME "awcotn")

set(LIB_LIB
    awcotn
    pthread
    ${YAMLCPP}
    )

message("***",${LIB_LIB})
    
add_executable(test tests/test.cc)
add_dependencies(test awcotn)
force_redefine_file_macro_for_sources(test) #__FILE__
target_link_libraries(test ${LIB_LIB})

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config awcotn)
force_redefine_file_macro_for_sources(test_config) #__FILE__
target_link_libraries(test_config ${LIB_LIB})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread awcotn)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread ${LIB_LIB})

add_executable(test_util tests/test_util.cc)
add_dependencies(test_util awcotn)
force_redefine_file_macro_for_sources(test_util) #__FILE__
target_link_libraries(test_util ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
