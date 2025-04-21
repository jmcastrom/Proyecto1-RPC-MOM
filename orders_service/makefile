# Makefile para el servicio de órdenes gRPC

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -g
LDFLAGS = -lgrpc++ -lgrpc -lprotobuf -lpthread

# Directorio fuente
SRC_DIR = .

# Archivos generados por protoc
PROTO_SRCS = orders_services.pb.cc orders_services.grpc.pb.cc \
             inventory_client.pb.cc inventory_client.grpc.pb.cc
PROTO_HDRS = orders_servicess.pb.h orders_services.grpc.pb.h \
             inventory_client.pb.h inventory_client.grpc.pb.h

# Archivos fuente del proyecto
SRCS = $(SRC_DIR)/main.cc $(SRC_DIR)/orders_service.cc
OBJS = $(SRCS:$(SRC_DIR)/%.cc=%.o) $(PROTO_SRCS:.cc=.o)

# Nombre del ejecutable
TARGET = orders_server

# Regla por defecto
all: proto $(TARGET)

# Compila los archivos .proto
proto: $(PROTO_SRCS)

%.pb.cc %.pb.h: %.proto
	protoc -I$(SRC_DIR) --cpp_out=$(SRC_DIR) $<

%.grpc.pb.cc %.grpc.pb.h: %.proto
	protoc -I$(SRC_DIR) --grpc_out=$(SRC_DIR) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

# Compilación de los archivos fuente
%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Enlazado final
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(PROTO_SRCS) $(PROTO_HDRS) $(TARGET)

# Forzar la recompilación de los protobuffers
clean-proto:
	rm -f $(PROTO_SRCS) $(PROTO_HDRS)

# Ejecutar el servidor
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean clean-proto run proto