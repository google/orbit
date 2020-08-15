// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_STATUS_LISTENER_IMPL_H_
#define ORBIT_QT_STATUS_LISTENER_IMPL_H_

#include <QStatusBar>
#include <cstdint>
#include <memory>
#include <string_view>

#include "StatusListener.h"
#include "absl/container/flat_hash_map.h"

class StatusListenerImpl : public StatusListener {
 public:
  [[nodiscard]] uint64_t AddStatus(std::string message) override;
  void ClearStatus(uint64_t status_id) override;
  void UpdateStatus(uint64_t status_id, std::string message) override;

  static std::unique_ptr<StatusListener> Create(QStatusBar* status_bar);

 private:
  explicit StatusListenerImpl(QStatusBar* status_bar) : next_id_{0}, status_bar_(status_bar) {}
  uint64_t GetNextId();

  uint64_t next_id_;
  absl::flat_hash_map<uint64_t, std::string> status_messages_;

  // Container holds recently updated status_id on the top
  // (at the end of the vector).
  std::vector<uint64_t> status_id_stack_;

  QStatusBar* status_bar_;
};

#endif  // ORBIT_QT_STATUS_LISTENER_IMPL_H_
