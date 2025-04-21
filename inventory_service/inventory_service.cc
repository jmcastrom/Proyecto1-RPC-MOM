#include "inventory_service.h"
#include <iostream>
#include <algorithm>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


InventoryServiceImpl::InventoryServiceImpl() {
  LoadSampleData();
}
/*
void InventoryServiceImpl::LoadSampleData() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Add some sample books
  books_["b1"] = {"b1", "Don Quixote", "Miguel de Cervantes", "9780099469698", 10, 15.99};
  books_["b2"] = {"b2", "1984", "George Orwell", "9780451524935", 15, 12.99};
  books_["b3"] = {"Harper Leb3", "To Kill a Mockingbird", "e", "9780061120084", 8, 14.50};
  books_["b4"] = {"b4", "The Great Gatsby", "F. Scott Fitzgerald", "9780743273565", 12, 11.99};
  books_["b5"] = {"b5", "One Hundred Years of Solitude", "Gabriel García Márquez", "9780060883287", 5, 16.75};
}*/

void InventoryServiceImpl::LoadSampleData() {
  std::lock_guard<std::mutex> lock(mutex_);

  inventory::Book b1;
  b1.set_book_id("b1");
  b1.set_title("Don Quixote");
  b1.set_author("Miguel de Cervantes");
  b1.set_isbn("9780099469698");
  b1.set_stock(10);
  b1.set_price(15.99);
  books_["b1"] = b1;

  inventory::Book b2;
  b2.set_book_id("b2");
  b2.set_title("1984");
  b2.set_author("George Orwell");
  b2.set_isbn("9780451524935");
  b2.set_stock(15);
  b2.set_price(12.99);
  books_["b2"] = b2;

  inventory::Book b3;
  b3.set_book_id("b3");
  b3.set_title("To Kill a Mockingbird");
  b3.set_author("Harper Lee");
  b3.set_isbn("9780061120084");
  b3.set_stock(8);
  b3.set_price(14.50);
  books_["b3"] = b3;

  inventory::Book b4;
  b4.set_book_id("b4");
  b4.set_title("The Great Gatsby");
  b4.set_author("F. Scott Fitzgerald");
  b4.set_isbn("9780743273565");
  b4.set_stock(12);
  b4.set_price(11.99);
  books_["b4"] = b4;

  inventory::Book b5;
  b5.set_book_id("b5");
  b5.set_title("One Hundred Years of Solitude");
  b5.set_author("Gabriel García Márquez");
  b5.set_isbn("9780060883287");
  b5.set_stock(5);
  b5.set_price(16.75);
  books_["b5"] = b5;
}


/*
grpc::Status InventoryServiceImpl::GetBook(grpc::ServerContext* context,
                                         const inventory::BookRequest* request,
                                         inventory::BookResponse* response) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  const auto& book_id = request->book_id();
  auto it = books_.find(book_id);
  
  if (it == books_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Book not found");
  }
  
  const auto& book = it->second;
  response->set_book_id(book.book_id);
  response->set_title(book.title);
  response->set_author(book.author);
  response->set_isbn(book.isbn);
  response->set_stock(book.stock);
  response->set_price(book.price);
  
  return grpc::Status::OK;
}
*/

grpc::Status InventoryServiceImpl::GetBook(grpc::ServerContext* context,
                                           const inventory::BookRequest* request,
                                           inventory::BookResponse* response) {
  std::cerr << "[InventoryService] GetBook recibido - book_id: " << request->book_id() << std::endl;
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = books_.find(request->book_id());
  if (it == books_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Book not found");
  }

  const inventory::Book& book = it->second;
  response->set_book_id(book.book_id());
  response->set_title(book.title());
  response->set_author(book.author());
  response->set_isbn(book.isbn());
  response->set_stock(book.stock());
  response->set_price(book.price());

  return grpc::Status::OK;
}


grpc::Status InventoryServiceImpl::SearchBooks(grpc::ServerContext* context,
                                               const inventory::SearchRequest* request,
                                               inventory::SearchResponse* response) {
  std::cerr << "[InventoryService] SearchBooks recibido - query: " << request->query()
          << ", page: " << request->page() << ", page_size: " << request->page_size() << std::endl;

  std::lock_guard<std::mutex> lock(mutex_);

  std::string query = request->query();

  for (const auto& pair : books_) {
    const auto& book = pair.second;
    if (book.title().find(query) != std::string::npos ||
        book.author().find(query) != std::string::npos) {
      inventory::BookResponse* result = response->add_books();
      result->set_book_id(book.book_id());
      result->set_title(book.title());
      result->set_author(book.author());
      result->set_isbn(book.isbn());
      result->set_stock(book.stock());
      result->set_price(book.price());
    }
  }

  response->set_total_results(response->books_size());
  return grpc::Status::OK;
}



grpc::Status InventoryServiceImpl::UpdateStock(grpc::ServerContext* context,
                                               const inventory::UpdateStockRequest* request,
                                               inventory::UpdateStockResponse* response) {
  std::cerr << "[InventoryService] UpdateStock recibido - book_id: " << request->book_id()
          << ", quantity_change: " << request->quantity_change() << std::endl;

  std::lock_guard<std::mutex> lock(mutex_);
  const std::string& book_id = request->book_id();

  auto it = books_.find(book_id);
  if (it == books_.end()) {
    response->set_success(false);
    response->set_message("Book not found");
    return grpc::Status::OK;
  }

  int new_stock = it->second.stock() + request->quantity_change();
  if (new_stock < 0) {
    response->set_success(false);
    response->set_message("Stock cannot be negative");
    return grpc::Status::OK;
  }

  it->second.set_stock(new_stock);
  response->set_success(true);
  response->set_new_stock(new_stock);
  response->set_message("Stock updated successfully");

  SaveToFile("inventory.json");
  return grpc::Status::OK;
}


/* 
grpc::Status InventoryServiceImpl::SearchBooks(grpc::ServerContext* context,
                                             const inventory::SearchRequest* request,
                                             inventory::SearchResponse* response) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  const auto& query = request->query();
  std::vector<Book> matching_books;
  
  // Simple search implementation that looks for the query in title or author
  for (const auto& pair : books_) {
    const auto& book = pair.second;
    if (book.title.find(query) != std::string::npos || 
        book.author.find(query) != std::string::npos ||
        book.isbn.find(query) != std::string::npos) {
      matching_books.push_back(book);
    }
  }
  
  // Pagination
  int page = std::max(1, request->page());
  int page_size = std::max(1, request->page_size());
  int start_idx = (page - 1) * page_size;
  
  response->set_total_results(matching_books.size());
  
  for (int i = start_idx; i < start_idx + page_size && i < matching_books.size(); ++i) {
    const auto& book = matching_books[i];
    auto* book_response = response->add_books();
    book_response->set_book_id(book.book_id);
    book_response->set_title(book.title);
    book_response->set_author(book.author);
    book_response->set_isbn(book.isbn);
    book_response->set_stock(book.stock);
    book_response->set_price(book.price);
  }
  
  return grpc::Status::OK;
}

grpc::Status InventoryServiceImpl::UpdateStock(grpc::ServerContext* context,
                                             const inventory::UpdateStockRequest* request,
                                             inventory::UpdateStockResponse* response) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  const auto& book_id = request->book_id();
  auto it = books_.find(book_id);
  
  if (it == books_.end()) {
    response->set_success(false);
    response->set_message("Book not found");
    return grpc::Status::OK;
  }
  
  auto& book = it->second;
  int new_stock = book.stock + request->quantity_change();
  
  if (new_stock < 0) {
    response->set_success(false);
    response->set_message("Not enough stock available");
    response->set_new_stock(book.stock);
    return grpc::Status::OK;
  }
  
  book.stock = new_stock;
  response->set_success(true);
  response->set_new_stock(new_stock);
  response->set_message("Stock updated successfully");
  
  return grpc::Status::OK;
}
*/

grpc::Status InventoryServiceImpl::AddBook(grpc::ServerContext* context, const inventory::Book* request, inventory::BookResponse* response) {
  std::lock_guard<std::mutex> lock(mutex_);

  const std::string& book_id = request->book_id();

  if (books_.count(book_id) > 0) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Book already exists");
  }

  books_[book_id] = *request;
  SaveToFile("inventory.json"); // guardar persistencia

  response->set_book_id(request->book_id());
  response->set_title(request->title());
  response->set_author(request->author());
  response->set_isbn(request->isbn());
  response->set_stock(request->stock());
  response->set_price(request->price());

  return grpc::Status::OK;
}

grpc::Status InventoryServiceImpl::DeleteBook(grpc::ServerContext* context, const inventory::BookRequest* request, inventory::UpdateStockResponse* response) {
  std::lock_guard<std::mutex> lock(mutex_);

  const std::string& book_id = request->book_id();
  auto it = books_.find(book_id);
  if (it == books_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Book not found");
  }

  books_.erase(it);
  SaveToFile("inventory.json"); // persistir cambio

  response->set_success(true);
  response->set_new_stock(0);
  response->set_message("Book deleted successfully");

  return grpc::Status::OK;
}

void InventoryServiceImpl::LoadFromFile(const std::string& filename) {
  std::lock_guard<std::mutex> lock(mutex_);

  std::ifstream file(filename);
  if (!file.is_open()) return;

  nlohmann::json j;
  file >> j;

  for (const auto& item : j.items()) {
    inventory::Book book;
    book.set_book_id(item.key());
    book.set_title(item.value().at("title").get<std::string>());
    book.set_author(item.value().at("author").get<std::string>());
    book.set_isbn(item.value().at("isbn").get<std::string>());
    book.set_stock(item.value().at("stock").get<int>());
    book.set_price(item.value().at("price").get<double>());

    books_[item.key()] = book;
  }
}


void InventoryServiceImpl::SaveToFile(const std::string& filename) {
  json j;

  for (const auto& pair : books_) {
    const auto& book = pair.second;
    j[pair.first] = {
      {"title", book.title()},
      {"author", book.author()},
      {"isbn", book.isbn()},
      {"stock", book.stock()},
      {"price", book.price()}
    };
  }

  std::ofstream file(filename);
  file << j.dump(4);
  file.close();
}
