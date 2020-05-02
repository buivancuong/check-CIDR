cmake_minimum_required(VERSION 3.16)
project(subnet_extraction)

set(CMAKE_CXX_STANDARD 14)

add_executable(subnet_extraction main.cpp)
add_executable(cidr_check cidr_check.cpp)
add_executable(cidr_server cidr_server.cpp)
add_executable(cidr_client cidr_client.cpp)