cmake_minimum_required(VERSION 3.15)
project(task2_hash)

set(CMAKE_CXX_STANDARD 14)

add_executable(task2_hash hash.c)

target_link_libraries(${PROJECT_NAME} crypto)

