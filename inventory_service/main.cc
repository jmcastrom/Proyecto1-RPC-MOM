// main.cc
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "inventory_service.h"

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  InventoryServiceImpl service;
  
  // Cargar inventario desde JSON
  service.LoadFromFile("inventory.json");


  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Inventory Service listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
