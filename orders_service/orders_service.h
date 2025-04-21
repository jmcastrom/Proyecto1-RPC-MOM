// orders_service.h
#pragma once
#include <grpcpp/channel.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "orders_service.grpc.pb.h"
#include "inventory_client.grpc.pb.h"

class OrdersServiceImpl final : public orders::OrdersService::Service {
public:
  OrdersServiceImpl(std::shared_ptr<grpc::Channel> inventory_channel);
  
  grpc::Status CreateOrder(grpc::ServerContext* context,
                         const orders::CreateOrderRequest* request,
                         orders::OrderResponse* response) override;
  
  grpc::Status GetOrder(grpc::ServerContext* context,
                      const orders::GetOrderRequest* request,
                      orders::OrderResponse* response) override;
  
  grpc::Status ListOrdersByUser(grpc::ServerContext* context,
                              const orders::ListOrdersRequest* request,
                              orders::ListOrdersResponse* response) override;

private:
  struct Order {
    std::string order_id;
    std::string user_id;
    std::vector<orders::OrderItem> items;
    double total_amount;
    std::string shipping_address;
    std::string status;
    std::string created_at;
  };
  
  std::unordered_map<std::string, Order> orders_;
  std::unordered_map<std::string, std::vector<std::string>> user_orders_;
  std::mutex mutex_;
  
  // Cliente para el servicio de inventario
  std::unique_ptr<inventory::InventoryService::Stub> inventory_stub_;
  
  std::string GenerateOrderId();
  std::string GetCurrentTimestamp();
};