#include "crow.h"
#include <cpr/cpr.h>
#include "inventory/inventory_client.h"

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/books")([]() {
        auto response = cpr::Get(cpr::Url{"http://3.94.127.0:8082/books"});
        crow::json::wvalue result;

        if (response.status_code == 200) {
            result = crow::json::load(response.text);
            //return crow::response{result.dump()};
	    return crow::response(result); 
	    //return crow::response(result);
        } else {
            result["error"] = "No se pudo obtener la información de libros";
            //return crow::response{500, result.dump()};
	    return crow::response(500, result);
            //return crow::response(500, crow::json::dump(result));
        }
    });

    // Rutas REST -> gRPC Inventory
    CROW_ROUTE(app, "/inventory/get")([](const crow::request& req) {
    	auto book_id = req.url_params.get("book_id") ? req.url_params.get("book_id") : "b1";

    	InventoryClient client(grpc::CreateChannel("54.163.165.4:50051", grpc::InsecureChannelCredentials()));
    	std::string response = client.GetBook(book_id);

   	 return crow::response(response);
    });



    app.bindaddr("0.0.0.0").port(8081).multithreaded().run();
}

