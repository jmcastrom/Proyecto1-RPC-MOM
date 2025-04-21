#pragma once
#include "mom.grpc.pb.h"
#include "orders_service.grpc.pb.h"
#include <grpcpp/grpcpp.h>

std::string HandleOrderRequest(const mom::MOMRequest& request) {
    auto channel = grpc::CreateChannel("52.4.93.36:50054", grpc::InsecureChannelCredentials());
    auto stub = orders::OrdersService::NewStub(channel);

    if (request.operation() == "GetOrder") {
        orders::GetOrderRequest req;
        req.set_order_id(request.order_id());
        orders::OrderResponse res;
        grpc::ClientContext ctx;

        auto status = stub->GetOrder(&ctx, req, &res);
        if (!status.ok()) return "Error: " + status.error_message();

        std::ostringstream oss;
        oss << "Order ID: " << res.order_id() << ", Total: $" << res.total_amount();
        return oss.str();
    }

    return "Operación de órdenes no implementada aún";
}
