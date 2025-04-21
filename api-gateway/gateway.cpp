#include "crow.h"
#include "mom.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <sstream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

int main() {
    crow::SimpleApp app;

    auto stub = mom::MomService::NewStub(
        grpc::CreateChannel("54.211.68.126:50055", grpc::InsecureChannelCredentials())
    );

    // INVENTORY

    CROW_ROUTE(app, "/inventory/get")([&stub](const crow::request& req) {
        std::cerr << "[API] POST /inventory/get" << std::endl;
	auto book_id = req.url_params.get("book_id") ?: "";

        mom::BookRequest request;
        request.set_book_id(book_id);

        mom::BookResponse response;
        ClientContext context;

        Status status = stub->GetBook(&context, request, &response);
        if (!status.ok()) return crow::response(500, "Error en GetBook");

        crow::json::wvalue res;
        res["book_id"] = response.book_id();
        res["title"] = response.title();
        res["author"] = response.author();
        res["isbn"] = response.isbn();
        res["stock"] = response.stock();
        res["price"] = response.price();
        return crow::response(res);
    });

    CROW_ROUTE(app, "/inventory/search")([&stub](const crow::request& req) {
	std::cerr << "[API] POST /inventory/search" << std::endl;
        auto query = req.url_params.get("query") ?: "";

        mom::SearchRequest request;
        request.set_query(query);
        request.set_page(1);
        request.set_page_size(10);

        mom::SearchResponse response;
        ClientContext context;

        Status status = stub->SearchBooks(&context, request, &response);
        if (!status.ok()) return crow::response(500, "Error en SearchBooks");

        crow::json::wvalue res;
        for (int i = 0; i < response.books_size(); ++i) {
            const auto& book = response.books(i);
            res[i]["book_id"] = book.book_id();
            res[i]["title"] = book.title();
            res[i]["author"] = book.author();
            res[i]["isbn"] = book.isbn();
            res[i]["stock"] = book.stock();
            res[i]["price"] = book.price();
        }
        return crow::response(res);
    });

    // USERS

    CROW_ROUTE(app, "/users/register").methods("POST"_method)([&stub](const crow::request& req) {
	std::cerr << "[API] POST /users/register" << std::endl;
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "JSON inválido");

        mom::RegisterRequest request;
        request.set_username(body["username"].s());
        request.set_email(body["email"].s());
        request.set_password(body["password"].s());
        request.set_full_name(body["full_name"].s());
        request.set_address(body["address"].s());

        mom::UserResponse response;
        ClientContext context;

        Status status = stub->RegisterUser(&context, request, &response);
        //return status.ok() ? crow::response("Usuario registrado") : crow::response(500, "Error en RegisterUser");
	if (status.ok()) {
            crow::json::wvalue result;
            result["message"] = "Usuario registrado";
            result["user_id"] = response.user_id();  // Aquí obtienes el ID
            return crow::response(result);
        } else {
            return crow::response(500, "Error en RegisterUser");
        }

    });

    CROW_ROUTE(app, "/users/authenticate").methods("POST"_method)([&stub](const crow::request& req) {
	std::cerr << "[API] POST /users/register" << std::endl;	 
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "JSON inválido");

        mom::AuthRequest request;
        request.set_username(body["username"].s());
        request.set_password(body["password"].s());

        mom::AuthResponse response;
        ClientContext context;
	
	grpc::Status status = stub->AuthenticateUser(&context, request, &response);

        return status.ok() ? crow::response(response.message()) : crow::response(401, "Credenciales incorrectas");
    });

    CROW_ROUTE(app, "/users/get")([&stub](const crow::request& req) {
	std::cerr << "[API] POST /users/get" << std::endl;
        auto user_id = req.url_params.get("user_id") ?: "";

        mom::GetUserRequest request;
        request.set_user_id(user_id);

        mom::UserResponse response;
        ClientContext context;

        Status status = stub->GetUser(&context, request, &response);
        if (!status.ok()) return crow::response(500, "Error en GetUser");

        crow::json::wvalue res;
        res["user_id"] = response.user_id();
        res["username"] = response.username();
        res["email"] = response.email();
        res["full_name"] = response.full_name();
        res["address"] = response.address();
        res["created_at"] = response.created_at();
        return crow::response(res);
    });

    CROW_ROUTE(app, "/users/update").methods("POST"_method)([&stub](const crow::request& req) {
	std::cerr << "[API] POST /users/update" << std::endl;
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "JSON inválido");

        mom::UpdateUserRequest request;
        request.set_user_id(body["user_id"].s());
        request.set_email(body["email"].s());
        request.set_full_name(body["full_name"].s());
        request.set_address(body["address"].s());

        mom::UserResponse response;
        ClientContext context;

        Status status = stub->UpdateUser(&context, request, &response);
        return status.ok() ? crow::response("Usuario actualizado") : crow::response(500, "Error en UpdateUser");
    });

    // ORDERS

    CROW_ROUTE(app, "/orders/create").methods("POST"_method)([&stub](const crow::request& req) {
	std::cerr << "[API] POST /users/create" << std::endl;
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "JSON inválido");

        mom::CreateOrderRequest request;
        request.set_user_id(body["user_id"].s());
        request.set_shipping_address(body["shipping_address"].s());

        for (auto& item : body["items"]) {
            auto* order_item = request.add_items();
            order_item->set_book_id(item["book_id"].s());
            order_item->set_quantity(item["quantity"].i());
            order_item->set_unit_price(item["unit_price"].d());
        }

        mom::OrderResponse response;
        ClientContext context;

        Status status = stub->CreateOrder(&context, request, &response);
        return status.ok() ? crow::response(response.order_id()) : crow::response(500, "Error en CreateOrder");
    });

    CROW_ROUTE(app, "/orders/get")([&stub](const crow::request& req) {
	std::cerr << "[API] POST /orders/get" << std::endl;
        auto order_id = req.url_params.get("order_id") ?: "";

        mom::GetOrderRequest request;
        request.set_order_id(order_id);

        mom::OrderResponse response;
        ClientContext context;

        Status status = stub->GetOrder(&context, request, &response);
        return status.ok() ? crow::response(response.DebugString()) : crow::response(500, "Error en GetOrder");
    });

    CROW_ROUTE(app, "/orders/list")([&stub](const crow::request& req) {
	std::cerr << "[API] POST /orders/get" << std::endl;
        auto user_id = req.url_params.get("user_id") ?: "";

        mom::ListOrdersRequest request;
        request.set_user_id(user_id);
        request.set_page(1);
        request.set_page_size(10);

        mom::ListOrdersResponse response;
        ClientContext context;

        Status status = stub->ListOrdersByUser(&context, request, &response);
        return status.ok() ? crow::response(response.DebugString()) : crow::response(500, "Error en ListOrdersByUser");
    });

    app.port(8081).bindaddr("0.0.0.0").multithreaded().run();
}
