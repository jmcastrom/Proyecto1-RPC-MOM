// inventory_service.h
#pragma once

#include <nlohmann/json.hpp>
#include <fstream>


#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "inventory_service.grpc.pb.h"

using json = nlohmann::json;

class InventoryServiceImpl final : public inventory::InventoryService::Service {
public:
  void LoadFromFile(const std::string& filename);
  void SaveToFile(const std::string& filename);
  InventoryServiceImpl();
  
  grpc::Status GetBook(grpc::ServerContext* context, 
                      const inventory::BookRequest* request,
                      inventory::BookResponse* response) override;
  
  grpc::Status SearchBooks(grpc::ServerContext* context,
                          const inventory::SearchRequest* request,
                          inventory::SearchResponse* response) override;
  
  grpc::Status UpdateStock(grpc::ServerContext* context,
                          const inventory::UpdateStockRequest* request,
                          inventory::UpdateStockResponse* response) override;

  grpc::Status AddBook(grpc::ServerContext* context, const inventory::Book* request, inventory::BookResponse* response) override;
  grpc::Status DeleteBook(grpc::ServerContext* context, const inventory::BookRequest* request, inventory::UpdateStockResponse* response) override;

private:
  /*struct Book {
    std::string book_id;
    std::string title;
    std::string author;
    std::string isbn;
    int stock;
    double price;
  };*/
  //std::unordered_map<std::string, inventory::Book> inventory_;  
  std::unordered_map<std::string, inventory::Book> books_;
  // std::unordered_map<std::string, Book> books_;
  std::mutex mutex_; // For thread safety
 
  //std::unordered_map<std::string, inventory::BookResponse> books_;
  //std::mutex mutex_;
  const std::string inventory_file_ = "inventory.json";
 
  void LoadSampleData();
  
};
