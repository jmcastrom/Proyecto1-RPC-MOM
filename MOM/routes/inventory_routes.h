#pragma once
#include "mom.grpc.pb.h"
#include "inventory_service.grpc.pb.h"
#include <grpcpp/grpcpp.h>

std::string HandleInventoryRequest(const mom::MOMRequest& request) {
    auto channel = grpc::CreateChannel("54.163.165.4:50051", grpc::InsecureChannelCredentials());
    auto stub = inventory::InventoryService::NewStub(channel);

    if (request.operation() == "GetBook") {
        inventory::BookRequest req;
        req.set_book_id(request.book_id());
        inventory::BookResponse res;
        grpc::ClientContext ctx;

        auto status = stub->GetBook(&ctx, req, &res);
        if (!status.ok()) return "Error: " + status.error_message();

        std::ostringstream oss;
        oss << "Book: " << res.title() << " by " << res.author()
            << " (ID: " << res.book_id() << ", Stock: " << res.stock()
            << ", Price: $" << res.price() << ")";
        return oss.str();
    }

    return "Operación de inventario no implementada aún";
}
