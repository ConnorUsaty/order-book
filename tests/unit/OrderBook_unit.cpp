#include <gtest/gtest.h>

#include "OrderBook.h"

class OrderBookUnitTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(OrderBookUnitTest, AddOrder) {
  OrderBook book;

  Order ord1(5, Side::Buy, 105, 4, 1);
  book.add_order(ord1);

  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.asks.size(), 0);

  Order ord2(6, Side::Buy, 105, 4, 2);
  book.add_order(ord2);

  EXPECT_EQ(book.order_pool.size(), 2);
  EXPECT_EQ(book.order_level.size(), 2);
  EXPECT_EQ(book.bids.size(), 1);
  EXPECT_EQ(book.asks.size(), 0);

  Order ord3(7, Side::Sell, 105, 10, 3);
  book.add_order(ord3);

  EXPECT_EQ(book.order_pool.size(), 1);
  EXPECT_EQ(book.order_level.size(), 1);
  EXPECT_EQ(book.bids.size(), 0);
  EXPECT_EQ(book.asks.size(), 1);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
