cmake_policy(SET CMP0048 NEW)

project(Lab1-myFTP)
cmake_minimum_required(VERSION 3.10)
enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(ftp_server ftp_server.cpp)
target_compile_options(ftp_server PUBLIC -Ofast)

add_executable(ftp_client ftp_client.cpp)
target_compile_options(ftp_client PUBLIC -Ofast)

include_directories(.)
target_link_libraries(ftp_server pthread)