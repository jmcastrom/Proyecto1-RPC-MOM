#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>  //  Reflexión

#include "proto/mom.grpc.pb.h"
#include "routes/mom_routes.h"

void RunServer() {
    std::string server_address("0.0.0.0:50055");  // Puerto del MOM
    MomServiceImpl service;

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();  //  Agrega esto

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "🟢 MOM listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
