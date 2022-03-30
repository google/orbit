// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TRACEPOINT_INFO_H_
#define CLIENT_DATA_TRACEPOINT_INFO_H_

#include <string>
#include <utility>

namespace orbit_client_data {

// This class is used on the client to represent a unique tracepoint, containing the tracepoint's
// name and category.
class TracepointInfo {
 public:
  TracepointInfo() = delete;
  TracepointInfo(std::string category, std::string name)
      : category_{std::move(category)}, name_{std::move(name)} {}

  [[nodiscard]] const std::string& category() const { return category_; }
  [[nodiscard]] const std::string& name() const { return name_; }

 private:
  std::string category_;
  std::string name_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TRACEPOINT_INFO_H_
