cmake_minimum_required(VERSION 3.15)
project(task3_block)

set(CMAKE_CXX_STANDARD 14)

add_executable(task3_block block.cpp)

target_link_libraries(${PROJECT_NAME} crypto)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME block)

