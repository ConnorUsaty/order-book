cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(GTest REQUIRED)

enable_testing()

add_executable(OrderBook_unit tests/unit/OrderBook_unit.cpp)
target_include_directories(OrderBook_unit PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(OrderBook_unit GTest::gtest GTest::gtest_main)

add_test(NAME OrderBook_unit COMMAND OrderBook_unit)
