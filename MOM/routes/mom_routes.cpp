// mom_routes.cpp
#include "mom_routes.h"

#include "inventory_service.grpc.pb.h"
#include "users.grpc.pb.h"
#include "orders_service.grpc.pb.h"

#include <grpcpp/create_channel.h>

#define INVENTORY_ADDR "54.163.165.4:50051"
#define USERS_ADDR "54.225.200.222:50053"
#define ORDERS_ADDR "52.4.93.36:50052"

// =================== INVENTORY ===================

grpc::Status MomServiceImpl::GetBook(grpc::ServerContext*, const mom::BookRequest* request, mom::BookResponse* response) {
    auto stub = inventory::InventoryService::NewStub(grpc::CreateChannel(INVENTORY_ADDR, grpc::InsecureChannelCredentials()));
    std::cerr << "[MOM] GetBook → book_id: " << request->book_id() << std::endl;
    grpc::ClientContext ctx;

    inventory::BookRequest invReq;
    invReq.set_book_id(request->book_id());

    inventory::BookResponse invResp;
    auto status = stub->GetBook(&ctx, invReq, &invResp);

    if (status.ok()) {
        response->set_book_id(invResp.book_id());
        response->set_title(invResp.title());
        response->set_author(invResp.author());
        response->set_isbn(invResp.isbn());
        response->set_stock(invResp.stock());
        response->set_price(invResp.price());
        return grpc::Status::OK;
    }
    return grpc::Status(grpc::StatusCode::INTERNAL, "Inventory: GetBook failed");
}

grpc::Status MomServiceImpl::SearchBooks(grpc::ServerContext*, const mom::SearchRequest* request, mom::SearchResponse* response) {
    auto stub = inventory::InventoryService::NewStub(grpc::CreateChannel(INVENTORY_ADDR, grpc::InsecureChannelCredentials()));
    std::cerr << "[MOM] SearchBooks → query: " << request->query() << std::endl;
    grpc::ClientContext ctx;

    inventory::SearchRequest invReq;
    invReq.set_query(request->query());
    invReq.set_page(request->page());
    invReq.set_page_size(request->page_size());

    inventory::SearchResponse invResp;
    auto status = stub->SearchBooks(&ctx, invReq, &invResp);

    if (status.ok()) {
        for (const auto& b : invResp.books()) {
            mom::BookResponse* out = response->add_books();
            out->set_book_id(b.book_id());
            out->set_title(b.title());
            out->set_author(b.author());
            out->set_isbn(b.isbn());
            out->set_stock(b.stock());
            out->set_price(b.price());
        }
        response->set_total_results(invResp.total_results());
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, "Inventory: SearchBooks failed");
}

// =================== USERS ===================
grpc::Status MomServiceImpl::RegisterUser(grpc::ServerContext*, const mom::RegisterRequest* request, mom::UserResponse* response) {
    std::cerr << "[MOM] → Recibida solicitud RegisterUser: " << request->username() << std::endl;

    auto stub = users::UsersService::NewStub(grpc::CreateChannel(USERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    users::RegisterRequest req;
    req.set_username(request->username());
    req.set_email(request->email());
    req.set_password(request->password());
    req.set_full_name(request->full_name());
    req.set_address(request->address());

    users::UserResponse userResp;
    auto status = stub->Register(&ctx, req, &userResp);

    if (status.ok()) {
        response->set_user_id(userResp.user_id());
        response->set_username(userResp.username());
        response->set_email(userResp.email());
        response->set_full_name(userResp.full_name());
        response->set_address(userResp.address());
        response->set_created_at(userResp.created_at());
        return grpc::Status::OK;
    }

    std::cerr << "[MOM] ❌ Error en RegisterUser: " << status.error_message() << std::endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}

grpc::Status MomServiceImpl::AuthenticateUser(grpc::ServerContext*, const mom::AuthRequest* request, mom::AuthResponse* response) {
    std::cerr << "[MOM] → Recibida solicitud AuthenticateUser: " << request->username() << std::endl;

    auto stub = users::UsersService::NewStub(grpc::CreateChannel(USERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    users::AuthRequest req;
    req.set_username(request->username());
    req.set_password(request->password());

    users::AuthResponse authResp;
    auto status = stub->Authenticate(&ctx, req, &authResp);

    if (status.ok()) {
        response->set_success(authResp.success());
        response->set_user_id(authResp.user_id());
        response->set_token(authResp.token());
        response->set_message(authResp.message());
        return grpc::Status::OK;
    }

    std::cerr << "[MOM] ❌ Error en AuthenticateUser: " << status.error_message() << std::endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}


grpc::Status MomServiceImpl::GetUser(grpc::ServerContext*, const mom::GetUserRequest* request, mom::UserResponse* response) {
    std::cerr << "[MOM] GetUser → user_id: " << request->user_id() << std::endl;
    auto stub = users::UsersService::NewStub(grpc::CreateChannel(USERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    users::GetUserRequest req;
    req.set_user_id(request->user_id());

    users::UserResponse userResp;
    auto status = stub->GetUser(&ctx, req, &userResp);

    if (status.ok()) {
        response->set_user_id(userResp.user_id());
        response->set_username(userResp.username());
        response->set_email(userResp.email());
        response->set_full_name(userResp.full_name());
        response->set_address(userResp.address());
        response->set_created_at(userResp.created_at());
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}


grpc::Status MomServiceImpl::UpdateUser(grpc::ServerContext*, const mom::UpdateUserRequest* request, mom::UserResponse* response) {
    std::cerr << "[MOM] UpdateUser → user_id: " << request->user_id() << std::endl;
    auto stub = users::UsersService::NewStub(grpc::CreateChannel(USERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    users::UpdateUserRequest req;
    req.set_user_id(request->user_id());
    req.set_email(request->email());
    req.set_full_name(request->full_name());
    req.set_address(request->address());

    users::UserResponse userResp;
    auto status = stub->UpdateUser(&ctx, req, &userResp);

    if (status.ok()) {
        response->set_user_id(userResp.user_id());
        response->set_username(userResp.username());
        response->set_email(userResp.email());
        response->set_full_name(userResp.full_name());
        response->set_address(userResp.address());
        response->set_created_at(userResp.created_at());
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}

// =================== ORDERS ===================
grpc::Status MomServiceImpl::CreateOrder(grpc::ServerContext*, const mom::CreateOrderRequest* request, mom::OrderResponse* response) {
    std::cerr << "[MOM] CreateOrder → user_id: " << request->user_id()
          << ", items: " << request->items_size() << std::endl;

    auto stub = orders::OrdersService::NewStub(grpc::CreateChannel(ORDERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    orders::CreateOrderRequest req;
    req.set_user_id(request->user_id());
    req.set_shipping_address(request->shipping_address());

    for (const auto& item : request->items()) {
        auto* out = req.add_items();
        out->set_book_id(item.book_id());
        out->set_quantity(item.quantity());
        out->set_unit_price(item.unit_price());
    }

    orders::OrderResponse orderResp;
    auto status = stub->CreateOrder(&ctx, req, &orderResp);

    if (status.ok()) {
        response->set_order_id(orderResp.order_id());
        response->set_user_id(orderResp.user_id());
        response->set_total_amount(orderResp.total_amount());
        response->set_shipping_address(orderResp.shipping_address());
        response->set_status(orderResp.status());
        response->set_created_at(orderResp.created_at());

        for (const auto& item : orderResp.items()) {
            auto* out = response->add_items();
            out->set_book_id(item.book_id());
            out->set_quantity(item.quantity());
            out->set_unit_price(item.unit_price());
        }

        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}

grpc::Status MomServiceImpl::GetOrder(grpc::ServerContext*, const mom::GetOrderRequest* request, mom::OrderResponse* response) {
    std::cerr << "[MOM] GetOrder → order_id: " << request->order_id() << std::endl;
    auto stub = orders::OrdersService::NewStub(grpc::CreateChannel(ORDERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    orders::GetOrderRequest req;
    req.set_order_id(request->order_id());

    orders::OrderResponse orderResp;
    auto status = stub->GetOrder(&ctx, req, &orderResp);

    if (status.ok()) {
        response->set_order_id(orderResp.order_id());
        response->set_user_id(orderResp.user_id());
        response->set_total_amount(orderResp.total_amount());
        response->set_shipping_address(orderResp.shipping_address());
        response->set_status(orderResp.status());
        response->set_created_at(orderResp.created_at());

        for (const auto& item : orderResp.items()) {
            auto* out = response->add_items();
            out->set_book_id(item.book_id());
            out->set_quantity(item.quantity());
            out->set_unit_price(item.unit_price());
        }

        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}

grpc::Status MomServiceImpl::ListOrdersByUser(grpc::ServerContext*, const mom::ListOrdersRequest* request, mom::ListOrdersResponse* response) {
    std::cerr << "[MOM] ListOrdersByUser → user_id: " << request->user_id()
          << ", page: " << request->page() << ", page_size: " << request->page_size() << std::endl;
    auto stub = orders::OrdersService::NewStub(grpc::CreateChannel(ORDERS_ADDR, grpc::InsecureChannelCredentials()));
    grpc::ClientContext ctx;

    orders::ListOrdersRequest req;
    req.set_user_id(request->user_id());
    req.set_page(request->page());
    req.set_page_size(request->page_size());

    orders::ListOrdersResponse listResp;
    auto status = stub->ListOrdersByUser(&ctx, req, &listResp);

    if (status.ok()) {
        response->set_total_orders(listResp.total_orders());
        for (const auto& ord : listResp.orders()) {
            mom::OrderResponse* out = response->add_orders();
            out->set_order_id(ord.order_id());
            out->set_user_id(ord.user_id());
            out->set_total_amount(ord.total_amount());
            out->set_shipping_address(ord.shipping_address());
            out->set_status(ord.status());
            out->set_created_at(ord.created_at());

            for (const auto& item : ord.items()) {
                auto* order_item = out->add_items();
                order_item->set_book_id(item.book_id());
                order_item->set_quantity(item.quantity());
                order_item->set_unit_price(item.unit_price());
            }
        }

        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INTERNAL, status.error_message());
}
