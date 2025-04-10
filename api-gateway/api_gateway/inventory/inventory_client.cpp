#include "inventory_client.h"

#include <sstream>

using grpc::ClientContext;
using grpc::Status;
using inventory::BookRequest;
using inventory::BookResponse;

InventoryClient::InventoryClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(inventory::InventoryService::NewStub(channel)) {}

std::string InventoryClient::GetBook(const std::string& book_id) {
    BookRequest request;
    request.set_book_id(book_id);

    BookResponse response;
    ClientContext context;

    Status status = stub_->GetBook(&context, request, &response);

    if (status.ok()) {
        std::ostringstream output;
        output << "Book: " << response.title() << " by " << response.author()
               << " (ID: " << response.book_id() << ", Stock: " << response.stock()
               << ", Price: $" << response.price() << ")";
        return output.str();
    } else {
        return "RPC failed: " + status.error_message();
    }
}
