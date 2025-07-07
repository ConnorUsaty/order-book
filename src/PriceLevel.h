/* A price level is a collection of orders at the same price */

#pragma once

#include <deque>

#include "Order.h"

struct PriceLevel {
  std::deque<Order*> orders;  // process orders in FIFO
  price_t level{0};
  quantity_t total_quantity{0};  // sum(all orders.quantity on this level)
};
