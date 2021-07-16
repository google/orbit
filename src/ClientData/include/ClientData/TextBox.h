// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TEXT_BOX_H_
#define CLIENT_DATA_TEXT_BOX_H_

#include <utility>

#include "capture_data.pb.h"

namespace orbit_client_data {
class TextBox {
 public:
  TextBox() = default;
  explicit TextBox(orbit_client_protos::TimerInfo timer_info)
      : timer_info_{std::move(timer_info)} {}

  // Delete the copy- and move-assignment operators, while keeping the copy- and move- constructors.
  // This is so that an element in TimerChain cannot just be re-assigned, which would break the
  // invariance on TimerBlock::min_timestamp_ and max_timestamp_.
  TextBox(const TextBox& other) = default;
  TextBox& operator=(const TextBox& other) = delete;
  TextBox(TextBox&& other) = default;
  TextBox& operator=(TextBox&& other) = delete;

  void SetSize(std::pair<float, float> size) { size_ = std::move(size); }
  void SetPos(std::pair<float, float> pos) { pos_ = std::move(pos); }

  [[nodiscard]] const std::pair<float, float>& GetSize() const { return size_; }
  [[nodiscard]] const std::pair<float, float>& GetPos() const { return pos_; }

  [[nodiscard]] const orbit_client_protos::TimerInfo& GetTimerInfo() const { return timer_info_; }

  // Start() and End() are required in order to be used as node in a ScopeTree.
  [[nodiscard]] uint64_t Start() const { return timer_info_.start(); }
  [[nodiscard]] uint64_t End() const { return timer_info_.end(); }
  [[nodiscard]] uint64_t Duration() const { return End() - Start(); }

 protected:
  orbit_client_protos::TimerInfo timer_info_;
  std::pair<float, float> pos_ = {0, 0};
  std::pair<float, float> size_ = {0, 0};
};
}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TEXT_BOX_H_
