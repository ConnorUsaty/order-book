/* Collection of all orders and all price levels */

#pragma once

#include <unordered_map>
#include <map>
#include "price_level.h"

class OrderBook {
    std::unordered_map<OrderId, Order> order_pool;
    std::unordered_map<OrderId, PriceLevel*> order_level;

    // store bids and asks in an ordered format for efficient processing
    std::map<PriceLevel, std::vector<PriceLevel>, std::greater<>> bids;
    std::map<PriceLevel, std::vector<PriceLevel>, std::less<>> asks;
};
