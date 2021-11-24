// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMER_INFOS_ITERATOR_H_
#define ORBIT_GL_TIMER_INFOS_ITERATOR_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"

class TimerInfosIterator {
 public:
  explicit TimerInfosIterator(
      std::vector<std::shared_ptr<orbit_client_data::TimerChain>>::const_iterator begin,
      std::vector<std::shared_ptr<orbit_client_data::TimerChain>>::const_iterator end)
      : chains_it_(begin),
        chains_end_it_(end),
        blocks_it_((chains_it_ != chains_end_it_) ? (*chains_it_)->begin()
                                                  : orbit_client_data::TimerChainIterator(nullptr)),
        timer_index_(0) {}

  TimerInfosIterator& operator=(const TimerInfosIterator& other) = default;
  TimerInfosIterator(const TimerInfosIterator& other) = default;
  TimerInfosIterator(TimerInfosIterator&& other) = default;
  TimerInfosIterator& operator=(TimerInfosIterator&& other) = default;

  TimerInfosIterator& operator++();

  const orbit_client_protos::TimerInfo& operator*() const { return (*blocks_it_)[timer_index_]; }

  const orbit_client_protos::TimerInfo* operator->() const { return &(*blocks_it_)[timer_index_]; }

  bool operator==(const TimerInfosIterator& other) const {
    return chains_it_ == other.chains_it_ && blocks_it_ == other.blocks_it_ &&
           timer_index_ == other.timer_index_;
  }
  bool operator!=(const TimerInfosIterator& other) const { return !(other == *this); }

 private:
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>>::const_iterator chains_it_;
  std::vector<std::shared_ptr<orbit_client_data::TimerChain>>::const_iterator chains_end_it_;
  orbit_client_data::TimerChainIterator blocks_it_;
  uint32_t timer_index_;
};

#endif  // ORBIT_GL_TIMER_INFOS_ITERATOR_H_
