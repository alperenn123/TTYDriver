cmake_minimum_required(VERSION 3.13)

project(MyttyTest)

include_directories(
    ${PROJECT_SOURCE_DIR}/
    ${/usr/include/}
)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES main.cpp serial.cpp)
add_executable(test_app ${SOURCES})
target_link_options(test_app PUBLIC "-lpthread") 
