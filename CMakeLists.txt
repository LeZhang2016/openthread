cmake_minimum_required(VERSION 3.3)
project(openthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_custom_target(openthread COMMAND make -C ${openthread_SOURCE_DIR}
        CLION_EXE_DIR=${PROJECT_BINARY_DIR})