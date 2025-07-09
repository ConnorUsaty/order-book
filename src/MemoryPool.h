/* Allows for more efficient memory allocation and acquisition for orders */

#pragma once

#include <array>
#include <memory>
#include <queue>
#include <type_traits>
#include <vector>

template <typename T>
class MemoryPool {
 private:
  struct Block {
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, 1024> storage;
    std::vector<T*> available;

    Block() {
      for (size_t i = 0; i < storage.size(); ++i) {
        available.push_back(reinterpret_cast<T*>(&storage[i]));
      }
    }
  };

  std::vector<std::unique_ptr<Block>> blocks;
  std::vector<T*> available;

 public:
  // vectors initialize empty
  MemoryPool() = default;

  MemoryPool(const MemoryPool&) = delete;
  MemoryPool& operator=(const MemoryPool&) = delete;
  MemoryPool(MemoryPool&&) = default;
  MemoryPool& operator=(MemoryPool&&) = default;

  template <typename... Args>
  T* acquire(Args&&... args) {
    if (available.empty()) {
      blocks.push_back(std::make_unique<Block>());
      available = blocks.back()->available;
    }

    T* ptr = available.back();
    available.pop_back();
    return new (ptr) T(std::forward<Args>(args)...);
  }

  void release(T* obj) {
    if (!obj) return;
    obj->~T();
    available.push_back(obj);
  }
};