#include <gtest/gtest.h>

#include "OrderBook.h"

class OrderBookUnitTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(OrderBookUnitTest, AddOrderFullMatch) {
  OrderBook book;

  Order ord1(1, Side::Buy, 105, 4, 1);
  book.add_order(ord1);
  auto& acc_ord1 = book.order_pool[1];

  EXPECT_EQ(acc_ord1, ord1);
  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.asks.size(), 0);

  Order ord2(2, Side::Buy, 105, 4, 2);
  book.add_order(ord2);
  auto& acc_ord2 = book.order_pool[2];

  EXPECT_EQ(acc_ord2, ord2);
  EXPECT_EQ(book.order_pool.size(), 2);
  EXPECT_EQ(book.order_level.size(), 2);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.asks.size(), 0);

  // full match + extra
  // covers the 8 buy orders and leaves 2 extra
  Order ord3(3, Side::Sell, 105, 10, 3);
  book.add_order(ord3);
  auto& acc_ord3 = book.order_pool[3];

  EXPECT_EQ(acc_ord3, ord3);
  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 0);
  EXPECT_EQ(book.asks.size(), 1);
}

TEST_F(OrderBookUnitTest, AddOrderPartialMatch) {
  OrderBook book;

  // sell 10 @ 100
  Order sell(1, Side::Sell, 100, 10, 1);
  book.add_order(sell);

  // buy 6 @ 100 -> partial match
  Order buy(2, Side::Buy, 100, 6, 2);
  book.add_order(buy);

  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 0);
  EXPECT_EQ(book.asks.size(), 1);

  auto& remaining_order = book.order_pool.begin()->second;
  EXPECT_EQ(remaining_order.id, 1);
  EXPECT_EQ(remaining_order.side, Side::Sell);
  EXPECT_EQ(remaining_order.price, 100);
  EXPECT_EQ(remaining_order.quantity, 4);
  EXPECT_EQ(remaining_order.timestamp, 1);
}

TEST_F(OrderBookUnitTest, AddOrderNoMatch) {
  OrderBook book;

  // Buy at 100, Sell at 102 -> no match
  Order buy(1, Side::Buy, 100, 10, 1);
  Order sell(2, Side::Sell, 102, 10, 2);

  book.add_order(buy);
  book.add_order(sell);

  EXPECT_EQ(book.order_pool.size(), 2);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.asks.size(), 1);
}

TEST_F(OrderBookUnitTest, CancelOrderBasic) {
  OrderBook book;

  Order buy(1, Side::Buy, 100, 10, 1);
  book.add_order(buy);

  // verify initial state
  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_pool.begin()->first, buy.id);
  EXPECT_EQ(book.order_pool.begin()->second, buy);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.bids.begin()->first, 100);
  EXPECT_EQ(book.bids.begin()->second.orders.size(), 1);
  // EXPECT_EQ(*(book.bids.begin()->second.orders.begin()), buy);
  EXPECT_EQ(book.bids.begin()->second.level, 100);
  EXPECT_EQ(book.bids.begin()->second.total_quantity, 10);
  EXPECT_EQ(book.asks.size(), 0);

  book.remove_order(2);
  book.remove_order(3);
  book.remove_order(4);

  // verify nothing changed
  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_pool.begin()->first, buy.id);
  EXPECT_EQ(book.order_pool.begin()->second, buy);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.bids.begin()->first, 100);
  EXPECT_EQ(book.bids.begin()->second.orders.size(), 1);
  // EXPECT_EQ(*(book.bids.begin()->second.orders.begin()), buy);
  EXPECT_EQ(book.bids.begin()->second.level, 100);
  EXPECT_EQ(book.bids.begin()->second.total_quantity, 10);
  EXPECT_EQ(book.asks.size(), 0);

  book.remove_order(1);

  // verify order removed
  EXPECT_EQ(book.order_pool.size(), 0);
  EXPECT_EQ(book.order_level.size(), 0);
  EXPECT_EQ(book.bids.size(), 0);
  EXPECT_EQ(book.asks.size(), 0);
}

TEST_F(OrderBookUnitTest, CancelOrderComplex) {
  OrderBook book;

  Order buy1(1, Side::Buy, 105, 10, 1);
  Order buy2(2, Side::Buy, 105, 20, 2);
  Order buy3(3, Side::Buy, 105, 15, 3);
  Order buy4(4, Side::Buy, 103, 25, 4);
  Order sell1(5, Side::Sell, 107, 30, 5);
  Order sell2(6, Side::Sell, 107, 10, 6);
  Order sell3(7, Side::Sell, 110, 5, 7);

  book.add_order(buy1);
  book.add_order(buy2);
  book.add_order(buy3);
  book.add_order(buy4);
  book.add_order(sell1);
  book.add_order(sell2);
  book.add_order(sell3);

  // verify initial state
  EXPECT_EQ(book.order_pool.size(), 7);
  EXPECT_EQ(book.bids.size(), 2);  // 105 and 103
  EXPECT_EQ(book.asks.size(), 2);  // 107 and 110
  EXPECT_EQ(book.bids[105].total_quantity, 45);
  EXPECT_EQ(book.bids[105].orders.size(), 3);
  EXPECT_EQ(book.asks[107].total_quantity, 40);

  // cancel first order at price 105
  book.cancel_order(1);

  EXPECT_EQ(book.order_pool.size(), 6);
  EXPECT_EQ(book.bids[105].total_quantity, 35);
  EXPECT_EQ(book.bids[105].orders.size(), 2);

  // verify order of remaining orders (FIFO)
  auto it = book.bids[105].orders.begin();
  EXPECT_EQ((*it)->id, 2);
  ++it;
  EXPECT_EQ((*it)->id, 3);

  // cancel last (idx not last at level) order at price 105
  book.cancel_order(3);

  EXPECT_EQ(book.order_pool.size(), 5);
  EXPECT_EQ(book.bids[105].total_quantity, 20);
  EXPECT_EQ(book.bids[105].orders.size(), 1);
  EXPECT_EQ(book.bids[105].orders.front()->id, 2);

  // cancel the only remaining order at price 105
  book.cancel_order(2);

  // price level 105 should be completely removed
  EXPECT_EQ(book.order_pool.size(), 4);
  EXPECT_EQ(book.bids.size(), 1);  // only 103 remains
  EXPECT_EQ(book.bids.find(105), book.bids.end());
  EXPECT_EQ(book.bids[103].total_quantity, 25);

  // cancel all remaining sell orders at 107
  book.cancel_order(5);
  book.cancel_order(6);

  EXPECT_EQ(book.order_pool.size(), 2);
  EXPECT_EQ(book.asks.size(), 1);  // only 110 remains
  EXPECT_EQ(book.asks.find(107), book.asks.end());
  EXPECT_EQ(book.asks[110].total_quantity, 5);

  // verify final state -> only orders 4 and 7 remain
  EXPECT_NE(book.order_pool.find(4), book.order_pool.end());
  EXPECT_NE(book.order_pool.find(7), book.order_pool.end());
  EXPECT_EQ(book.order_level.size(), 2);
}

TEST_F(OrderBookUnitTest, GetTopComplex) {
  OrderBook book;

  // verify empty book
  BookTop empty_top = book.get_top();
  EXPECT_EQ(empty_top.bid_price, 0);
  EXPECT_EQ(empty_top.bid_quantity, 0);
  EXPECT_EQ(empty_top.ask_price, 0);
  EXPECT_EQ(empty_top.ask_quantity, 0);
  EXPECT_EQ(empty_top.spread(), 0);
  EXPECT_EQ(empty_top.mid(), 0);

  // verify bid only
  Order buy1(1, Side::Buy, 100, 10, 1);
  Order buy2(2, Side::Buy, 99, 20, 2);
  Order buy3(3, Side::Buy, 100, 15, 3);
  book.add_order(buy1);
  book.add_order(buy2);
  book.add_order(buy3);

  BookTop bid_only = book.get_top();
  EXPECT_EQ(bid_only.bid_price, 100);
  EXPECT_EQ(bid_only.bid_quantity, 25);
  EXPECT_EQ(bid_only.ask_price, 0);
  EXPECT_EQ(bid_only.ask_quantity, 0);

  // add asks
  Order sell1(4, Side::Sell, 102, 30, 4);
  Order sell2(5, Side::Sell, 101, 5, 5);
  Order sell3(6, Side::Sell, 101, 10, 6);
  book.add_order(sell1);
  book.add_order(sell2);
  book.add_order(sell3);

  BookTop full_book = book.get_top();
  EXPECT_EQ(full_book.bid_price, 100);
  EXPECT_EQ(full_book.bid_quantity, 25);
  EXPECT_EQ(full_book.ask_price, 101);
  EXPECT_EQ(full_book.ask_quantity, 15);
  EXPECT_EQ(full_book.spread(), 1);
  EXPECT_EQ(full_book.mid(), 100);

  // verify partial match
  Order cross_buy(7, Side::Buy, 101, 12, 7);
  book.add_order(cross_buy);

  BookTop after_match = book.get_top();
  EXPECT_EQ(after_match.bid_price, 100);
  EXPECT_EQ(after_match.bid_quantity, 25);
  EXPECT_EQ(after_match.ask_price, 101);
  EXPECT_EQ(after_match.ask_quantity, 3);

  // verify removing best bid level
  book.cancel_order(1);
  book.cancel_order(3);  // removes entire 100 level

  BookTop new_best = book.get_top();
  EXPECT_EQ(new_best.bid_price, 99);  // new best bid
  EXPECT_EQ(new_best.bid_quantity, 20);
  EXPECT_EQ(new_best.ask_price, 101);
  EXPECT_EQ(new_best.ask_quantity, 3);
  EXPECT_EQ(new_best.spread(), 2);
  EXPECT_EQ(new_best.mid(), 100);

  // verify a wide spread
  Order sell_high(8, Side::Sell, 200, 100, 8);
  book.add_order(sell_high);
  book.cancel_order(5);
  book.cancel_order(6);

  BookTop wide_spread = book.get_top();
  EXPECT_EQ(wide_spread.bid_price, 99);
  EXPECT_EQ(wide_spread.ask_price, 102);
  EXPECT_EQ(wide_spread.spread(), 3);
  EXPECT_EQ(wide_spread.mid(), 100);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
