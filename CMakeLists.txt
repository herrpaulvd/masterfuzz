cmake_minimum_required(VERSION 3.0.0)
project(masterfuzz VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fsanitize=address")

add_subdirectory(DecoderBase)
add_subdirectory(Instances)
add_subdirectory(SimpleFuzzer)

add_executable(masterfuzz main.cpp)

target_link_libraries(masterfuzz Instances)
