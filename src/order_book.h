/* Collection of all orders and all price levels */

#pragma once

#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>

#include "price_level.h"

class OrderBook {
 public:
  void add_order(Order& order) {
    if (order.side == Side::Buy) {
      // try to match to sell orders -> need an ask with <= order.price
      while (!asks.empty() && asks.begin()->first <= order.price &&
             order.quantity > 0) {
        execute_match(order, asks.begin()->second);
      }
    } else {
      // try to match to a buy order -> need a bid with >= order.price
      while (!bids.empty() && bids.begin()->first >= order.price &&
             order.quantity > 0) {
        execute_match(order, bids.begin()->second);
      }
    }

    if (order.quantity > 0) {
      insert_order(order);
    }
  }

  void execute_match(Order& order, PriceLevel& price_level) {
    // executes a match from either side on a price_level
    // below conditions must hold and should be checked before calling
    if (order.side == Side::Buy) {
      assert(order.price >= price_level.level);
    } else {
      assert(order.price <= price_level.level);
    }

    // can use entire level
    if (price_level.total_quantity <= order.quantity) {
      order.quantity -= price_level.total_quantity;
      Side opp_side = (order.side == Side::Buy ? Side::Sell : Side::Buy);
      remove_level(price_level, opp_side);
      return;
    }

    // can use part of the level
    while (!price_level.orders.empty() && order.quantity > 0) {
      auto curr = price_level.orders.front();
      Quantity matched = std::min(curr->quantity, order.quantity);
      price_level.total_quantity -= matched;
      curr->quantity -= matched;
      order.quantity -= matched;
      if (curr->quantity == 0) price_level.orders.pop_front();
    }
  }

  void insert_order(const Order& order) {
    // inserts order into the OrderBook
    assert(order.quantity > 0);

    PriceLevel* price_level = nullptr;
    if (order.side == Side::Buy) {
      price_level = &(bids[order.price]);
    } else {
      price_level = &(asks[order.price]);
    }
    price_level->level = order.price;
    assert(price_level != nullptr);

    auto [it, inserted] = order_pool.insert({order.id, order});
    Order* stored_order =
        &(it->second);  // needed to get a pointer to the stored order
    order_level.insert({order.id, price_level});
    price_level->orders.push_back(stored_order);
    price_level->total_quantity += order.quantity;
  }

  void remove_order(const OrderId id) {
    // removes an order from the OrderBook
    // must still remove from PriceLevel
    order_pool.erase(id);
    order_level.erase(id);
  }

  void remove_level(PriceLevel& price_level, Side side) {
    for (auto& order : price_level.orders) {
      remove_order(order->id);
    }
    if (side == Side::Buy) {
      bids.erase(price_level.level);
    } else {
      asks.erase(price_level.level);
    }
  }

  std::unordered_map<OrderId, Order> order_pool;
  std::unordered_map<OrderId, PriceLevel*> order_level;

  // store bids and asks in an ordered format for efficient processing
  std::map<Price, PriceLevel, std::greater<>> bids;
  std::map<Price, PriceLevel, std::less<>> asks;
};
