/* A price level is a collection of orders at the same price */

#pragma once

#include <list>

#include "Order.h"

struct PriceLevel {
  std::list<Order*> orders;  // process orders in FIFO
  price_t level{0};
  quantity_t total_quantity{0};  // sum(all orders.quantity on this level)
};

// for usage with BookTop to access metrics of OrderBook
struct LevelInfo {
  price_t price;
  quantity_t quantity;
  size_t order_count;

  LevelInfo(price_t _price, quantity_t _quantity, size_t _order_count)
      : price(_price), quantity(_quantity), order_count(_order_count) {}
};
