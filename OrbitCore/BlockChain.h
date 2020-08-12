// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_BLOCK_CHAIN_H_
#define ORBIT_CORE_BLOCK_CHAIN_H_

#include <algorithm>
#include <cstdint>

#include "OrbitBase/Logging.h"

template <class T, uint32_t BlockSize>
class BlockChain;

template <class T, uint32_t BlockSize>
class BlockIterator;

template <class T, uint32_t Size>
class Block final {
 public:
  explicit Block(Block<T, Size>* prev)
      : prev_(prev), next_(nullptr), size_(0) {}

  [[nodiscard]] bool HasNext() const { return next_ != nullptr; }
  [[nodiscard]] const Block<T, Size>* next() const { return next_; }
  [[nodiscard]] const Block<T, Size>* prev() const { return prev_; }
  [[nodiscard]] uint32_t size() const { return size_; }

  [[nodiscard]] const T* data() const { return data_; }

 private:
  friend class BlockIterator<T, Size>;
  friend class BlockChain<T, Size>;

  [[nodiscard]] T& Get(uint32_t index) { return data_[index]; }
  [[nodiscard]] Block<T, Size>* mutable_next() { return next_; }
  [[nodiscard]] Block<T, Size>* mutable_prev() { return prev_; }

  void ResetSize() { size_ = 0; }

  void Reset() {
    size_ = 0;
    next_ = nullptr;
    prev_ = nullptr;
  }

  // Returns block the element was inserted to.
  template <typename V>
  Block<T, Size>* Add(V&& item) {
    if (size() == Size) {
      if (!HasNext()) {
        next_ = new Block<T, Size>(this);
      }

      return next_->Add(std::forward<V>(item));
    }

    CHECK(size_ < Size);
    data_[size_++] = std::forward<V>(item);

    return this;
  }

  Block<T, Size>* prev_;
  Block<T, Size>* next_;
  uint32_t size_;
  T data_[Size];
};

template <class T, uint32_t BlockSize>
class BlockIterator final {
 public:
  explicit BlockIterator(Block<T, BlockSize>* block) : block_(block) {
    index_ = (block_ != nullptr && block_->size() > 0) ? 0 : -1;
  }

  T& operator*() { return block_->Get(index_); }

  bool operator!=(const BlockIterator& other) const {
    if (block_ != other.block_) {
      return true;
    }

    return index_ != other.index_;
  }

  BlockIterator& operator++() {
    if (++index_ != block_->size()) {
      return *this;
    }

    if (block_->next() != nullptr && block_->next()->size() > 0) {
      index_ = 0;
      block_ = block_->mutable_next();
    } else {
      // end()
      block_ = nullptr;
      index_ = -1;
    }

    return *this;
  }

 private:
  Block<T, BlockSize>* block_;
  uint32_t index_;
};

template <class T, uint32_t BlockSize>
class BlockChain final {
 public:
  BlockChain() : size_(0) {
    root_ = current_ = new Block<T, BlockSize>(nullptr);
  }

  ~BlockChain() {
    // Find last block in chain
    while (current_->HasNext()) {
      current_ = current_->mutable_next();
    }

    Block<T, BlockSize>* prev = current_;
    while (prev) {
      prev = current_->mutable_prev();
      delete current_;
      current_ = prev;
    }
  }

  template <typename V>
  void push_back(V&& item) {
    current_ = current_->Add(std::forward<V>(item));
    ++size_;
  }

  void push_back(const T* array, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
      push_back(array[i]);
    }
  }

  void push_back_n(const T& item, uint32_t num) {
    for (uint32_t i = 0; i < num; ++i) {
      push_back(item);
    }
  }

  void clear() {
    root_->Reset();
    size_ = 0;

    Block<T, BlockSize>* prev = current_;
    while (prev != root_) {
      prev = current_->mutable_prev();
      delete current_;
      current_ = prev;
    }

    current_ = root_;
  }

  [[nodiscard]] const Block<T, BlockSize>* root() const { return root_; }

  void Reset() {
    Block<T, BlockSize>* block = root_;
    while (block) {
      block->ResetSize();
      block = block->mutable_next();
    }

    size_ = 0;
    current_ = root_;
  }

  [[nodiscard]] uint32_t size() const { return size_; }

  T* SlowAt(uint32_t index) {
    if (index > size_) {
      return nullptr;
    }

    uint32_t count = 1;
    Block<T, BlockSize>* block = root_;
    while (count * BlockSize < index && block && block->HasNext()) {
      block = block->mutable_next();
      ++count;
    }

    CHECK(block != nullptr);

    return &block->Get(index % BlockSize);
  }

  T* GetElementAfter(const T* element) {
    auto block = GetBlockContaining(element);
    if (!block) {
      return nullptr;
    }

    T* begin = &block->Get(0);
    uint32_t index = element - begin;

    if (index + 1 < block->size()) {
      return &block->Get(index + 1);
    }

    if (block->HasNext() && block->next()->size() != 0) {
      return &block->mutable_next()->Get(0);
    }

    return nullptr;
  }

  BlockIterator<T, BlockSize> begin() {
    return BlockIterator<T, BlockSize>(root_);
  }
  BlockIterator<T, BlockSize> end() {
    return BlockIterator<T, BlockSize>(nullptr);
  }

 private:
  Block<T, BlockSize>* GetBlockContaining(const T* element) {
    Block<T, BlockSize>* block = root_;
    while (block != nullptr) {
      uint32_t size = block->size();
      if (size != 0) {
        T* begin = &block->Get(0);
        T* end = &block->Get(size - 1);
        if (begin <= element && end >= element) {
          return block;
        }
      }
      block = block->mutable_next();
    }

    return nullptr;
  }

  Block<T, BlockSize>* root_;
  Block<T, BlockSize>* current_;
  uint32_t size_;
};

#endif  // ORBIT_CORE_BLOCK_CHAIN_H_