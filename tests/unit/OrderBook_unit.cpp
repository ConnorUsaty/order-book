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

TEST_F(OrderBookUnitTest, BasicCancelOrder) {
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
