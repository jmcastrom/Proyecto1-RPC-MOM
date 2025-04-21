# Nombre del ejecutable final
TARGET = inventory_server

# Archivos fuente
SRCS = main.cc inventory_service.cc inventory_service.pb.cc inventory_service.grpc.pb.cc

# Archivos generados por protoc
PROTO_SRC = inventory_service.pb.cc inventory_service.grpc.pb.cc
PROTO_HDR = inventory_service.pb.h inventory_service.grpc.pb.h

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 `pkg-config --cflags protobuf grpc++`
LDFLAGS = `pkg-config --libs protobuf grpc++`

# Regla por defecto
all: $(TARGET)

# Compila el ejecutable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Genera los archivos a partir del .proto
$(PROTO_SRC) $(PROTO_HDR): inventory_service.proto
	protoc -I=. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

# Limpia archivos generados
clean:
	rm -f $(TARGET) *.o $(PROTO_SRC) $(PROTO_HDR)
