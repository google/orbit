// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTAINERS_BLOCK_CHAIN_H_
#define CONTAINERS_BLOCK_CHAIN_H_

#include <algorithm>
#include <cstdint>

#include "OrbitBase/Logging.h"

namespace orbit_containers {

template <class T, uint32_t BlockSize>
class BlockChain;

template <class T, uint32_t BlockSize>
class BlockIterator;

template <class T, uint32_t Size>
class Block final {
 public:
  explicit Block(Block<T, Size>* prev) : prev_(prev), next_(nullptr) { data_.reserve(Size); }

  [[nodiscard]] bool HasNext() const { return next_ != nullptr; }
  [[nodiscard]] const Block<T, Size>* next() const { return next_; }
  [[nodiscard]] const Block<T, Size>* prev() const { return prev_; }
  [[nodiscard]] uint32_t size() const { return data_.size(); }
  [[nodiscard]] const T* data() const { return data_.data(); }
  [[nodiscard]] bool at_capacity() const { return size() == Size; }

 private:
  friend class BlockIterator<T, Size>;
  friend class BlockChain<T, Size>;

  [[nodiscard]] const T& Get(uint32_t index) const { return data_[index]; }
  [[nodiscard]] Block<T, Size>* mutable_next() { return next_; }
  [[nodiscard]] Block<T, Size>* mutable_prev() { return prev_; }

  void ResetSize() { data_.resize(0); }

  void Reset() {
    ResetSize();
    next_ = nullptr;
    prev_ = nullptr;
  }

  // Returns block the element was inserted to.
  template <class... Args>
  T& emplace_back(Args&&... args) {
    ORBIT_CHECK(size() < Size);
    return data_.emplace_back(std::forward<Args>(args)...);
  }

  Block<T, Size>* prev_;
  Block<T, Size>* next_;
  std::vector<T> data_;
};

template <class T, uint32_t BlockSize>
class BlockIterator final {
 public:
  explicit BlockIterator(const Block<T, BlockSize>* block)
      : block_(block), index_((block_ != nullptr && block_->size() > 0) ? 0 : -1) {}

  const T& operator*() const { return block_->Get(index_); }

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

    if (block_->HasNext() && block_->next()->size() > 0) {
      index_ = 0;
      block_ = block_->next();
    } else {
      // end()
      block_ = nullptr;
      index_ = -1;
    }

    return *this;
  }

 private:
  const Block<T, BlockSize>* block_;
  uint32_t index_;
};

template <class T, uint32_t BlockSize>
class BlockChain final {
 public:
  BlockChain() : size_(0) { root_ = current_ = new Block<T, BlockSize>(nullptr); }

  BlockChain(const BlockChain& other) = delete;

  BlockChain& operator=(const BlockChain& other) = delete;

  BlockChain(BlockChain&& other) = delete;

  BlockChain& operator=(BlockChain&& other) = delete;

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

  template <size_t size>
  void push_back(const std::array<T, size>& array) {
    for (uint32_t i = 0; i < size; ++i) {
      emplace_back(array[i]);
    }
  }

  void push_back_n(const T& item, uint32_t num) {
    for (uint32_t i = 0; i < num; ++i) {
      emplace_back(item);
    }
  }

  template <class... Args>
  T& emplace_back(Args&&... args) {
    if (current_->at_capacity()) AllocateOrRecycleBlock();
    T& new_item = current_->emplace_back(std::forward<Args>(args)...);
    ++size_;
    return new_item;
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

  [[nodiscard]] BlockIterator<T, BlockSize> begin() const {
    return size() > 0 ? BlockIterator<T, BlockSize>(root_) : end();
  }

  [[nodiscard]] BlockIterator<T, BlockSize> end() const {
    return BlockIterator<T, BlockSize>(nullptr);
  }

 private:
  void AllocateOrRecycleBlock() {
    if (current_->next_ == nullptr) {
      current_->next_ = new Block<T, BlockSize>(current_);
    }
    current_ = current_->next_;
  }

  Block<T, BlockSize>* root_;
  Block<T, BlockSize>* current_;
  uint32_t size_;
};

}  // namespace orbit_containers

#endif  // CONTAINERS_BLOCK_CHAIN_H_
