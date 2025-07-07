#include <cstdint>

using OrderId = uint64_t;
using Price = int64_t; // store price in cents as int
using Quantity = uint32_t;
using Timestamp = uint64_t;

enum class Side : uint8_t {
    Buy,
    Sell
};

// each order must have the following data
struct Order {
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
};
