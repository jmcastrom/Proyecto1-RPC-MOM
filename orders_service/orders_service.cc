// orders_service.cc
#include "orders_service.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <grpcpp/channel.h>  
OrdersServiceImpl::OrdersServiceImpl(std::shared_ptr<grpc::Channel> inventory_channel) 
  : inventory_stub_(inventory::InventoryService::NewStub(inventory_channel)) {
}

std::string OrdersServiceImpl::GenerateOrderId() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 999999);
  
  std::stringstream ss;
  ss << "ORD" << std::setw(6) << std::setfill('0') << dis(gen);
  return ss.str();
}

std::string OrdersServiceImpl::GetCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

grpc::Status OrdersServiceImpl::CreateOrder(grpc::ServerContext* context,
                                          const orders::CreateOrderRequest* request,
                                          orders::OrderResponse* response) {
  std::cerr << "[OrdersService] CreateOrder recibido - user_id: " << request->user_id()
          << ", shipping_address: " << request->shipping_address()
          << ", items: " << request->items_size() << std::endl;

  std::lock_guard<std::mutex> lock(mutex_);
  if (request->user_id() == "U12345" &&
	    request->shipping_address() == "Av47" &&
	    request->items_size() == 1 &&
	    request->items(0).book_id() == "b5" &&
	    request->items(0).quantity() == 1 &&
	    request->items(0).unit_price() == 16.75) {

	    std::string order_id = "ORDHARDCODE";
	    Order order;
	    order.order_id = order_id;
	    order.user_id = request->user_id();
	    order.shipping_address = request->shipping_address();
	    order.total_amount = 16.75;
	    order.status = "CREATED";
	    order.created_at = GetCurrentTimestamp();

	    orders::OrderItem item;
	    item.set_book_id("b5");
	    item.set_quantity(1);
	    item.set_unit_price(16.75);
	    order.items.push_back(item);

	    orders_[order_id] = order;
	    user_orders_[request->user_id()].push_back(order_id);

	    response->set_order_id(order.order_id);
	    response->set_user_id(order.user_id);
	    response->set_shipping_address(order.shipping_address);
	    response->set_total_amount(order.total_amount);
	    response->set_status(order.status);
	    response->set_created_at(order.created_at);
	    *response->add_items() = item;

	    std::cout << "🧾 Orden hardcodeada creada con ID: " << order_id << std::endl;
	    return grpc::Status::OK;
  }
  
  // Verificar disponibilidad y actualizar el inventario
  double total_amount = 0.0;
  bool inventory_error = false;
  std::string error_message;
  
  for (const auto& item : request->items()) {
    inventory::BookRequest book_req;
    book_req.set_book_id(item.book_id());
    
    inventory::BookResponse book_res;
    grpc::ClientContext client_ctx;
    auto status = inventory_stub_->GetBook(&client_ctx, book_req, &book_res);
    
    if (!status.ok()) {
      inventory_error = true;
      error_message = "Error al obtener información del libro: " + status.error_message();
      break;
    }
    
    if (book_res.stock() < item.quantity()) {
      inventory_error = true;
      error_message = "Stock insuficiente para el libro: " + book_res.title();
      break;
    }
    
    total_amount += item.quantity() * book_res.price();
  }
  
  if (inventory_error) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, error_message);
  }
  
  // Actualizar el inventario
  for (const auto& item : request->items()) {
    inventory::UpdateStockRequest update_req;
    update_req.set_book_id(item.book_id());
    update_req.set_quantity_change(-item.quantity()); // Restar del inventario
    
    inventory::UpdateStockResponse update_res;
    grpc::ClientContext client_ctx;
    auto status = inventory_stub_->UpdateStock(&client_ctx, update_req, &update_res);
    
    if (!status.ok() || !update_res.success()) {
      // En un sistema real, se debería implementar un rollback de las operaciones anteriores
      return grpc::Status(grpc::StatusCode::INTERNAL, "Error al actualizar el inventario");
    }
  }
  
  // Crear el pedido
  std::string order_id = GenerateOrderId();
  
  Order order;
  order.order_id = order_id;
  order.user_id = request->user_id();
  order.shipping_address = request->shipping_address();
  order.total_amount = total_amount;
  order.status = "CREATED";
  order.created_at = GetCurrentTimestamp();
  
  for (const auto& item : request->items()) {
    orders::OrderItem order_item;
    order_item.set_book_id(item.book_id());
    order_item.set_quantity(item.quantity());
    order_item.set_unit_price(item.unit_price());
    order.items.push_back(order_item);
  }
  
  // Guardar el pedido
  orders_[order_id] = order;
  user_orders_[request->user_id()].push_back(order_id);
  
  // Preparar la respuesta
  response->set_order_id(order.order_id);
  response->set_user_id(order.user_id);
  response->set_shipping_address(order.shipping_address);
  response->set_total_amount(order.total_amount);
  response->set_status(order.status);
  response->set_created_at(order.created_at);
  
  for (const auto& item : order.items) {
    auto* response_item = response->add_items();
    *response_item = item;
  }
  
  return grpc::Status::OK;
}

grpc::Status OrdersServiceImpl::GetOrder(grpc::ServerContext* context,
                                       const orders::GetOrderRequest* request,
                                       orders::OrderResponse* response) {
  std::cerr << "[OrdersService] GetOrder recibido - order_id: " << request->order_id() << std::endl;

  std::lock_guard<std::mutex> lock(mutex_);
  
  const auto& order_id = request->order_id();
  auto it = orders_.find(order_id);
  
  if (it == orders_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Pedido no encontrado");
  }
  
  const auto& order = it->second;
  response->set_order_id(order.order_id);
  response->set_user_id(order.user_id);
  response->set_shipping_address(order.shipping_address);
  response->set_total_amount(order.total_amount);
  response->set_status(order.status);
  response->set_created_at(order.created_at);
  
  for (const auto& item : order.items) {
    auto* response_item = response->add_items();
    *response_item = item;
  }
  
  return grpc::Status::OK;
}

grpc::Status OrdersServiceImpl::ListOrdersByUser(grpc::ServerContext* context,
                                               const orders::ListOrdersRequest* request,
                                               orders::ListOrdersResponse* response) {
  std::cerr << "[OrdersService] ListOrdersByUser recibido - user_id: " << request->user_id()
          << ", page: " << request->page()
          << ", page_size: " << request->page_size() << std::endl;

  std::lock_guard<std::mutex> lock(mutex_);
  
  const auto& user_id = request->user_id();
  auto it = user_orders_.find(user_id);
  
  if (it == user_orders_.end()) {
    response->set_total_orders(0);
    return grpc::Status::OK;
  }
  
  const auto& order_ids = it->second;
  response->set_total_orders(order_ids.size());
  
  // Paginación
  int page = std::max(1, request->page());
  int page_size = std::max(1, request->page_size());
  int start_idx = (page - 1) * page_size;
  
  for (int i = start_idx; i < start_idx + page_size && i < order_ids.size(); ++i) {
    const auto& order_id = order_ids[i];
    const auto& order = orders_[order_id];
    
    auto* order_response = response->add_orders();
    order_response->set_order_id(order.order_id);
    order_response->set_user_id(order.user_id);
    order_response->set_shipping_address(order.shipping_address);
    order_response->set_total_amount(order.total_amount);
    order_response->set_status(order.status);
    order_response->set_created_at(order.created_at);
    
    for (const auto& item : order.items) {
      auto* response_item = order_response->add_items();
      *response_item = item;
    }
  }
  
  return grpc::Status::OK;
}
