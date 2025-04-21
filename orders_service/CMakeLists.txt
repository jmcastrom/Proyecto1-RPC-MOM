cmake_minimum_required(VERSION 3.10)
project(orders_service)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Mensajes de depuración para verificar las rutas
message(STATUS "Protoc path: ${PROTOBUF_PROTOC_EXECUTABLE}")
message(STATUS "gRPC plugin path: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# Fuerza las rutas conocidas si CMake no las detecta correctamente
set(PROTOBUF_PROTOC_EXECUTABLE "/usr/bin/protoc")
set(GRPC_CPP_PLUGIN_EXECUTABLE "/usr/bin/grpc_cpp_plugin")

# Generar código a partir de los archivos proto
set(PROTO_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/inventory_client.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/orders_service.proto
)

foreach(PROTO_FILE ${PROTO_SOURCES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")
    
    add_custom_command(
        OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}"
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR} 
             --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
             -I ${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generando código gRPC para ${PROTO_FILE}"
    )
    
    list(APPEND GENERATED_SOURCES ${PROTO_SRC} ${GRPC_SRC})
endforeach()

# Ejecutable principal
add_executable(orders_server
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/orders_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/orders_service.h
    ${GENERATED_SOURCES}
)

target_include_directories(orders_server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(orders_server PRIVATE gRPC::grpc++ gRPC::grpc protobuf::libprotobuf)