/* Loop over a bunch of OrderBook ops and perf record to find bottlenecks */

#include <vector>

#include "OrderBook.h"

int main() {
  OrderBook book;

  size_t constexpr k_num_orders = 100000U;
  std::vector<Order> orders;
  orders.reserve(k_num_orders);
  for (size_t order = 0; order < k_num_orders; ++order) {
    if (order & 1) {
      orders.push_back(Order{order, Side::Buy, 2 * order, order, order});
    } else {
      orders.push_back(Order{order, Side::Sell, 2 * order, order, order});
    }
  }

  for (size_t order = 0; order < k_num_orders; ++order) {
    book.add_order(orders[order]);
    book.ask_depth();
    book.bid_depth();
    book.get_top();
    book.get_asks(book.ask_depth());
    book.get_bids(book.bid_depth());
  }

  (void)book;

  return 0;
}