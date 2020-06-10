// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_CHAIN_
#define ORBIT_GL_TIMER_CHAIN_

#include <assert.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>

#include "TextBox.h"

class TimerBlock {
 public:
  TimerBlock();

  bool AtCapacity() const {
    return size_ == kMaxSize;
  }

  // Silently fails when the block is full.
  void Add(const TextBox& box);
  
  bool Intersects(uint64_t min, uint64_t max);

  int size() const {
    return size_;
  }

  uint64_t min_timestamp() {
    return min_timestamp_;
  }

  uint64_t max_timestamp() {
    return max_timestamp_;
  }

  TextBox& operator[](std::size_t idx) { 
    return data_[idx];
  }
    
  const TextBox& operator[](std::size_t idx) const { 
    return data_[idx];
  }
  
 private:
  static constexpr int kMaxSize = 1024;
  TextBox data_[kMaxSize];
  int size_;

  uint64_t min_timestamp_;
  uint64_t max_timestamp_;
};

class TimerChain {
 public:
  TimerChain();

  void push_back(const TextBox& box);

  int size() const { return blocks_.size(); }

  TimerBlock& operator[](std::size_t idx) {
    return blocks_[idx];
  }
    
  const TimerBlock& operator[](std::size_t idx) const {
    return blocks_[idx];
  }

  int GetBlockContaining(const TextBox* element);

  TextBox* GetElementAfter(const TextBox* element);

  TextBox* GetElementBefore(const TextBox* element);

 private:
  std::vector<TimerBlock> blocks_;
};

#endif