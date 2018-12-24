cmake_minimum_required(VERSION 3.6)

project(CppProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(thirdparties/boost.cmake)
include(thirdparties/rangev3.cmake)

add_executable(CppApp main.cpp)

target_link_libraries(CppApp PRIVATE Boost::boost Boost::context range-v3)

