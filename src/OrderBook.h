/* Collection of all orders and all price levels */

#pragma once

#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>

#include "MemoryPool.h"
#include "PriceLevel.h"

// to easily access metrics of OrderBook
struct BookTop {
  price_t bid_price{0};
  quantity_t bid_quantity{0};
  price_t ask_price{0};
  quantity_t ask_quantity{0};

  BookTop() = default;

  inline price_t spread() const { return ask_price - bid_price; }
  inline price_t mid() const { return (ask_price + bid_price) / 2; }
};

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

  void modify_order(const order_id_t id, const quantity_t new_quantity) {
    if (new_quantity == 0) remove_order(id);

    auto it = order_pool.find(id);
    if (it == order_pool.end()) return;

    // update PriceLevel
    auto& [price_level, order_it] = order_level[id];
    price_level->total_quantity -= (it->second->quantity - new_quantity);

    // update OrderBook
    it->second->quantity = new_quantity;
  }

  void cancel_order(const order_id_t id) { remove_order(id); }

  BookTop get_top() const {
    BookTop top;
    if (!bids.empty()) {
      top.bid_price = bids.begin()->first;
      top.bid_quantity = bids.begin()->second.total_quantity;
    }
    if (!asks.empty()) {
      top.ask_price = asks.begin()->first;
      top.ask_quantity = asks.begin()->second.total_quantity;
    }
    return top;
  }

  inline size_t bid_depth() const { return bids.size(); }

  inline size_t ask_depth() const { return asks.size(); }

  std::vector<LevelInfo> get_bids(size_t max_depth) const {
    std::vector<LevelInfo> result;
    if (max_depth == 0) return result;
    result.reserve(std::min(max_depth, bid_depth()));

    for (auto const& [price, price_level] : bids) {
      result.push_back(LevelInfo{price, price_level.total_quantity,
                                 price_level.orders.size()});
      if (--max_depth == 0) break;
    }
    return result;
  }

  std::vector<LevelInfo> get_asks(size_t max_depth) const {
    std::vector<LevelInfo> result;
    if (max_depth == 0) return result;
    result.reserve(std::min(max_depth, ask_depth()));

    for (auto const& [price, price_level] : asks) {
      result.push_back(LevelInfo{price, price_level.total_quantity,
                                 price_level.orders.size()});
      if (--max_depth == 0) break;
    }
    return result;
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
      quantity_t matched = std::min(curr->quantity, order.quantity);
      price_level.total_quantity -= matched;
      curr->quantity -= matched;
      order.quantity -= matched;
      if (curr->quantity == 0) remove_order(curr->id);
    }
  }

  void insert_order(const Order& order) {
    // inserts order into the OrderBook and PriceLevel
    assert(order.quantity > 0);
    Order* pooled_order = mem_pool.acquire(order);

    PriceLevel* price_level = nullptr;
    if (pooled_order->side == Side::Buy) {
      price_level = &(bids[pooled_order->price]);
    } else {
      price_level = &(asks[pooled_order->price]);
    }
    price_level->level = pooled_order->price;
    assert(price_level != nullptr);

    order_pool.insert({pooled_order->id, pooled_order});
    price_level->orders.push_back(pooled_order);
    price_level->total_quantity += pooled_order->quantity;
    auto order_it = std::prev(price_level->orders.end());
    order_level.insert({pooled_order->id, {price_level, order_it}});
  }

  void remove_order(const order_id_t id) {
    auto it = order_pool.find(id);
    if (it == order_pool.end()) return;

    // remove from PriceLevel
    auto [level, order_it] = order_level[id];
    level->total_quantity -= (*order_it)->quantity;
    level->orders.erase(order_it);
    if (level->orders.empty()) {
      remove_level(*level, it->second->side);
    }

    // remove from OrderBook
    order_pool.erase(id);
    order_level.erase(id);

    // remove from MemoryPool
    mem_pool.release(*order_it);
  }

  void remove_level(PriceLevel& price_level, Side side) {
    for (auto& order : price_level.orders) {
      // remove from OrderBook
      order_pool.erase(order->id);
      order_level.erase(order->id);
      // remove from MemoryPool
      mem_pool.release(order);
    }

    // this deallocates the PriceLevel
    if (side == Side::Buy) {
      bids.erase(price_level.level);
    } else {
      asks.erase(price_level.level);
    }
  }

  MemoryPool<Order> mem_pool;
  std::unordered_map<order_id_t, Order*> order_pool;
  std::unordered_map<order_id_t,
                     std::pair<PriceLevel*, std::list<Order*>::iterator>>
      order_level;

  // store bids and asks in an ordered format for efficient processing
  std::map<price_t, PriceLevel, std::greater<>> bids;
  std::map<price_t, PriceLevel, std::less<>> asks;
};
