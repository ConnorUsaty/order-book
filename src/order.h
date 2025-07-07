#pragma once

#include <cstdint>

using order_id_t = uint64_t;
using price_t = int64_t;  // store price in cents as int
using quantity_t = uint32_t;
using timestamp_t = uint64_t;

enum class Side : uint8_t { Buy, Sell };

// each order must have the following data
struct Order {
  order_id_t id;
  Side side;
  price_t price;
  quantity_t quantity;
  timestamp_t timestamp;

  Order() = default;
  Order(order_id_t _id, Side _side, price_t _price, quantity_t _quantity,
        timestamp_t _timestamp)
      : id(_id),
        side(_side),
        price(_price),
        quantity(_quantity),
        timestamp(_timestamp) {}
};
