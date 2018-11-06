cmake_minimum_required(VERSION 3.10)

project(posits)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native")

add_executable(${PROJECT_NAME}
        src/posits.cc
        )

find_library(LIB_FLETCHER fletcher)
find_library(LIB_ARROW arrow)

target_link_libraries(${PROJECT_NAME}
        ${REQUIRED}
        ${LIB_ARROW}
        ${LIB_FLETCHER}
        )

