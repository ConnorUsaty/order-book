# order-book

A C++20 implementation of a financial exchange order book optimized for low latency and high throughput.

### Features
##### Core Functionality

- Price-Time Priority Matching: FIFO execution within each price level
- Order Types: Limit orders with aggressive execution
- Partial Fills: Support for partial order execution
- O(1) Order Cancellation and Modification: Achieved through std::list iterator caching

##### Performance Optimizations

- Zero-Allocation Design: Pre-allocated memory pools eliminate malloc/free in hot path
- Cache-Aligned Structures: Data structures aligned to 64-byte cache lines

##### Efficient Data Structures:

- std::map for sparse price level distribution
- std::list for O(1) order removal with iterator storage
- std::unordered_map for O(1) order lookup

##### Market Data

- Top of Book: O(1) access to best bid/ask with spread calculation
- Market Depth: Configurable depth snapshots with order counts
- Aggregated Quantities: Pre-calculated totals at each price level
