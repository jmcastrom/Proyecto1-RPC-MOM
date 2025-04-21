// main.cc
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "orders_service.h"

void RunServer() {
  std::string server_address("0.0.0.0:50052");
  std::string inventory_address("54.163.165.4:50051");
  
  // Crear canal para comunicarse con el servicio de inventario
  auto inventory_channel = grpc::CreateChannel(inventory_address, grpc::InsecureChannelCredentials());
  
  OrdersServiceImpl service(inventory_channel);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Orders Service listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
