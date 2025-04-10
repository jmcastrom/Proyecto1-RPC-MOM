cmake_minimum_required(VERSION 3.10)
project(gateway)

set(CMAKE_CXX_STANDARD 17)

# CPR y CROW deben estar disponibles
find_package(cpr REQUIRED)

include_directories(../crow/include)

add_executable(gateway gateway.cpp)
target_link_libraries(gateway PRIVATE cpr::cpr)
