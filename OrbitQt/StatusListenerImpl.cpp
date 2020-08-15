// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "StatusListenerImpl.h"

#include "OrbitBase/Logging.h"

uint64_t StatusListenerImpl::AddStatus(std::string message) {
  uint64_t id = GetNextId();
  status_bar_->showMessage(QString::fromStdString(message));

  status_messages_.insert_or_assign(id, std::move(message));
  status_id_stack_.push_back(id);

  return id;
}

void StatusListenerImpl::ClearStatus(uint64_t status_id) {
  CHECK(status_messages_.count(status_id) == 1);
  status_messages_.erase(status_id);
  status_id_stack_.erase(std::remove(status_id_stack_.begin(), status_id_stack_.end(), status_id),
                         status_id_stack_.end());
  if (status_id_stack_.empty()) {
    status_bar_->clearMessage();
    return;
  }

  uint64_t current_status_id = status_id_stack_.back();
  status_bar_->showMessage(QString::fromStdString(status_messages_.at(current_status_id)));
}

void StatusListenerImpl::UpdateStatus(uint64_t status_id, std::string message) {
  CHECK(status_messages_.count(status_id) == 1);
  auto it = std::find(status_id_stack_.begin(), status_id_stack_.end(), status_id);
  CHECK(it != status_id_stack_.end());

  status_id_stack_.erase(it);
  status_id_stack_.push_back(status_id);

  status_bar_->showMessage(QString::fromStdString(message));
  status_messages_.insert_or_assign(status_id, std::move(message));
}

std::unique_ptr<StatusListener> StatusListenerImpl::Create(QStatusBar* status_bar) {
  return std::unique_ptr<StatusListener>(new StatusListenerImpl(status_bar));
}

uint64_t StatusListenerImpl::GetNextId() {
  CHECK(next_id_ < std::numeric_limits<uint64_t>::max());
  return next_id_++;
}
