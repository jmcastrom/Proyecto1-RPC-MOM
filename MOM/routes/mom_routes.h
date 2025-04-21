#ifndef MOM_ROUTES_H
#define MOM_ROUTES_H

#include "mom.grpc.pb.h"

class MomServiceImpl final : public mom::MomService::Service {
public:
    // INVENTORY
    grpc::Status GetBook(grpc::ServerContext* context, const mom::BookRequest* request, mom::BookResponse* response) override;
    grpc::Status SearchBooks(grpc::ServerContext* context, const mom::SearchRequest* request, mom::SearchResponse* response) override;

    // USERS
    grpc::Status RegisterUser(grpc::ServerContext* context, const mom::RegisterRequest* request, mom::UserResponse* response) override;
    grpc::Status AuthenticateUser(grpc::ServerContext* context, const mom::AuthRequest* request, mom::AuthResponse* response) override;
    grpc::Status GetUser(grpc::ServerContext* context, const mom::GetUserRequest* request, mom::UserResponse* response) override;
    grpc::Status UpdateUser(grpc::ServerContext* context, const mom::UpdateUserRequest* request, mom::UserResponse* response) override;

    // ORDERS
    grpc::Status CreateOrder(grpc::ServerContext* context, const mom::CreateOrderRequest* request, mom::OrderResponse* response) override;
    grpc::Status GetOrder(grpc::ServerContext* context, const mom::GetOrderRequest* request, mom::OrderResponse* response) override;
    grpc::Status ListOrdersByUser(grpc::ServerContext* context, const mom::ListOrdersRequest* request, mom::ListOrdersResponse* response) override;
};

#endif // MOM_ROUTES_H
