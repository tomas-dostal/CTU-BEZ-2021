cmake_minimum_required(VERSION 3.15)
project(task4_asymmetric)

set(CMAKE_CXX_STANDARD 14)

add_executable(task4_asymmetric encrypt.cpp)

target_link_libraries(${PROJECT_NAME} crypto)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME encrypt)

