/* A price level is a collection of orders at the same price */

#pragma once

#include <deque>
#include "order.h"

struct PriceLevel {
    std::deque<Order*> orders; // process orders in FIFO
    Price level;
    Quantity total_quantity; // sum(all orders.quantity on this level)
};
