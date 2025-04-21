cmake_minimum_required(VERSION 3.12)
project(api_gateway_minimal)

set(CMAKE_CXX_STANDARD 17)

# Buscar Protobuf y gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Incluir headers
include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../crow/include
    #${CMAKE_CURRENT_SOURCE_DIR}/crow/include
)

# Archivos fuente
add_executable(gateway
    gateway.cpp
    proto/mom.pb.cc
    proto/mom.grpc.pb.cc
)

# Enlazar librerías
target_link_libraries(gateway
    ${Protobuf_LIBRARIES}
    gRPC::grpc++
)
