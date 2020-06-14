cmake_minimum_required(VERSION 3.10)
project(SimpleLogger)

set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT ON)
endif ()

add_library(SimpleLogger Sources/Logger.cpp)

target_compile_options(SimpleLogger PRIVATE -std=c++17 -Wextra -Werror -Wall)
target_include_directories(SimpleLogger INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Headers)

if (${MASTER_PROJECT})
    enable_testing()
endif()
add_subdirectory(Tests)
