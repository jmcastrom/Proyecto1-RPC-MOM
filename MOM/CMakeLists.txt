cmake_minimum_required(VERSION 3.14)
project(mom)

set(CMAKE_CXX_STANDARD 17)

# Encontrar Protobuf y gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Incluir directorios
include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/routes
)

# Archivos generados desde .proto
set(PROTO_SRCS
    proto/inventory_service.pb.cc
    proto/inventory_service.grpc.pb.cc
    proto/mom.pb.cc
    proto/mom.grpc.pb.cc
    proto/orders_service.pb.cc
    proto/orders_service.grpc.pb.cc
    proto/users.pb.cc
    proto/users.grpc.pb.cc
)

set(PROTO_HDRS
    proto/inventory_service.pb.h
    proto/inventory_service.grpc.pb.h
    proto/mom.pb.h
    proto/mom.grpc.pb.h
    proto/orders_service.pb.h
    proto/orders_service.grpc.pb.h
    proto/users.pb.h
    proto/users.grpc.pb.h
)

# Archivos fuente propios
set(SOURCE_FILES
    main.cpp
    routes/mom_routes.cpp
)

# Ejecutable final
add_executable(mom ${SOURCE_FILES} ${PROTO_SRCS} ${PROTO_HDRS})

# Vincular librerías
target_link_libraries(mom
    ${Protobuf_LIBRARIES}
    gRPC::grpc++
    gRPC::grpc++_reflection
)
