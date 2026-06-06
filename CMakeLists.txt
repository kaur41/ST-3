cmake_minimum_required(VERSION 3.10.2)
project(ST-3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tool_dest "bin")
set(lib_dest "lib")
set(include_dest "include/")

# Подключаем Google Test и Google Mock
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src)
add_subdirectory(test)