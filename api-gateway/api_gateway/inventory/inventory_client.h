#ifndef INVENTORY_CLIENT_H
#define INVENTORY_CLIENT_H

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "inventory_service.grpc.pb.h"
#include "inventory_service.pb.h"

class InventoryClient {
public:
    InventoryClient(std::shared_ptr<grpc::Channel> channel);
    std::string GetBook(const std::string& book_id);

private:
    std::unique_ptr<inventory::InventoryService::Stub> stub_;
};

#endif // INVENTORY_CLIENT_H
