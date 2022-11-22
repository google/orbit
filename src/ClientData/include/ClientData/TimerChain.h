// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_CHAIN_H_
#define CLIENT_DATA_TIMER_CHAIN_H_

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <utility>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

// TimerBlock is a straightforward specialization of Block (see BlockChain.h) with the added bonus
// that it keeps track of the minimum and maximum timestamps of all timers added to it. This allows
// trivial rejection of an entire block by using the Intersects(t_min, t_max) method. This
// effectively tests if any of the timers stored in this block intersects with the [t_min, t_max]
// interval.
class TimerBlock {
  friend class TimerChain;
  friend class TimerChainIterator;

 public:
  explicit TimerBlock(TimerBlock* prev)
      : prev_(prev),
        next_(nullptr),
        min_timestamp_(std::numeric_limits<uint64_t>::max()),
        max_timestamp_(std::numeric_limits<uint64_t>::min()) {
    data_.reserve(kBlockSize);
  }

  // Append a new element to the end of the block using placement-new.
  template <class... Args>
  const orbit_client_protos::TimerInfo& emplace_back(Args&&... args) {
    ORBIT_CHECK(size() < kBlockSize);
    const orbit_client_protos::TimerInfo& timer_info =
        data_.emplace_back(std::forward<Args>(args)...);
    min_timestamp_ = std::min(timer_info.start(), min_timestamp_);
    max_timestamp_ = std::max(timer_info.end(), max_timestamp_);
    return timer_info;
  }

  // Tests if [min, max] intersects with [min_timestamp, max_timestamp], where
  // {min, max}_timestamp are the minimum and maximum timestamp of the timers
  // that have so far been added to this block.
  [[nodiscard]] bool Intersects(uint64_t min, uint64_t max) const;
  [[nodiscard]] uint64_t MinTimestamp() const { return min_timestamp_; }

  [[nodiscard]] size_t size() const { return data_.size(); }
  [[nodiscard]] bool at_capacity() const { return size() == kBlockSize; }

  [[nodiscard]] const orbit_client_protos::TimerInfo& operator[](std::size_t idx) const {
    return data_[idx];
  }

  // Assuming timers are sorted, returns the first one for which the end timestamp isn't smaller
  // than min_ns. Return nullptr if there is none.
  [[nodiscard]] const orbit_client_protos::TimerInfo* LowerBound(uint64_t min_ns) const;

 private:
  static constexpr size_t kBlockSize = 1024;

  TimerBlock* prev_;
  TimerBlock* next_;
  std::vector<orbit_client_protos::TimerInfo> data_;

  uint64_t min_timestamp_;
  uint64_t max_timestamp_;
};  // TimerChainIterator iterates over all *blocks* of the chain, not the
// individual items (TimerInfo instances) that are stored in the blocks (this is
// different from the BlockIterator in BlockChain.h).
class TimerChainIterator {
 public:
  explicit TimerChainIterator(TimerBlock* block) : block_(block) {}
  TimerChainIterator(const TimerChainIterator& other) = default;
  TimerChainIterator& operator=(const TimerChainIterator& other) = default;
  TimerChainIterator(TimerChainIterator&& other) = default;
  TimerChainIterator& operator=(TimerChainIterator&& other) = default;

  bool operator!=(const TimerChainIterator& other) const { return !(*this == other); }

  bool operator==(const TimerChainIterator& other) const { return block_ == other.block_; }
  TimerChainIterator& operator++() {
    block_ = block_->next_;
    return *this;
  }

  const TimerBlock& operator*() const { return *block_; }
  const TimerBlock* operator->() const { return block_; }

 private:
  TimerBlock* block_;
};

// TimerChain is a specialization of the BlockChain data structure to make it
// easier to keep track of min and max timestamps in the blocks, which allows
// for fast rejection of entire blocks when rendering timers. Note that there
// is a difference compared with BlockChain in how the iterators work: Here,
// the iterator runs over blocks, in BlockChain the iterator runs over the
// individually stored elements.
class TimerChain {
 public:
  ~TimerChain();

  // Append an item to the end of the current block. If capacity of the current block is reached, a
  // new blocked is allocated and the item is added to the new block.
  template <class... Args>
  const orbit_client_protos::TimerInfo& emplace_back(Args&&... args) {
    if (current_->at_capacity()) AllocateNewBlock();
    const orbit_client_protos::TimerInfo& timer_info =
        current_->emplace_back(std::forward<Args>(args)...);
    ++num_items_;
    return timer_info;
  }

  [[nodiscard]] bool empty() const { return num_items_ == 0; }
  [[nodiscard]] uint64_t size() const { return num_items_; }

  [[nodiscard]] const TimerBlock* GetBlockContaining(
      const orbit_client_protos::TimerInfo& element) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetElementAfter(
      const orbit_client_protos::TimerInfo& element) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetElementBefore(
      const orbit_client_protos::TimerInfo& element) const;

  [[nodiscard]] TimerChainIterator begin() const { return TimerChainIterator(root_); }

  [[nodiscard]] TimerChainIterator end() const { return TimerChainIterator(nullptr); }

 private:
  void AllocateNewBlock() {
    ORBIT_CHECK(current_->next_ == nullptr);
    current_->next_ = new TimerBlock(current_);
    current_ = current_->next_;
    ++num_blocks_;
  }

  TimerBlock* root_ = new TimerBlock(/*prev=*/nullptr);
  TimerBlock* current_ = root_;
  uint64_t num_blocks_ = 1;
  uint64_t num_items_ = 0;
};
}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_CHAIN_H_
