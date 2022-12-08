// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/PickingManager.h"

#include <absl/base/casts.h>
#include <absl/synchronization/mutex.h>

#include <utility>

#include "OrbitBase/Logging.h"

PickingId PickingManager::GetOrCreatePickableId(const std::shared_ptr<Pickable>& pickable,
                                                BatcherId batcher_id) {
  absl::MutexLock lock(&mutex_);
  uint32_t pickable_id = 0;

  auto it = pickable_pid_map_.find(pickable.get());
  if (it == pickable_pid_map_.end()) {
    pickable_id = ++pickable_id_counter_;
    pid_pickable_map_[pickable_id] = pickable;
    pickable_pid_map_[pickable.get()] = pickable_id;
  } else {
    pickable_id = it->second;
  }

  PickingId id = PickingId::Create(PickingType::kPickable, pickable_id, batcher_id);
  return id;
}

void PickingManager::Reset() {
  absl::MutexLock lock(&mutex_);
  pid_pickable_map_.clear();
  pickable_pid_map_.clear();
  pickable_id_counter_ = 0;
}

std::shared_ptr<Pickable> PickingManager::GetPickableFromId(PickingId id) const {
  ORBIT_CHECK(id.type == PickingType::kPickable);

  absl::MutexLock lock(&mutex_);
  auto it = pid_pickable_map_.find(id.element_id);
  if (it == pid_pickable_map_.end()) {
    return nullptr;
  }
  return it->second.lock();
}

std::shared_ptr<Pickable> PickingManager::GetPicked() const {
  absl::MutexLock lock(&mutex_);
  return currently_picked_.lock();
}

void PickingManager::Pick(PickingId id, int x, int y) {
  auto picked = GetPickableFromId(id);
  if (picked) {
    picked->OnPick(x, y);
  }

  absl::MutexLock lock(&mutex_);
  currently_picked_ = picked;
}

void PickingManager::Release() {
  auto picked = GetPicked();
  if (picked != nullptr) {
    picked->OnRelease();
    absl::MutexLock lock(&mutex_);
    currently_picked_.reset();
  }
}

void PickingManager::Drag(int x, int y) const {
  auto picked = GetPicked();
  if (picked && picked->Draggable()) {
    picked->OnDrag(x, y);
  }
}

bool PickingManager::IsDragging() const {
  absl::MutexLock lock(&mutex_);
  auto picked = currently_picked_.lock();
  return picked && picked->Draggable();
}

Color PickingManager::GetPickableColor(const std::shared_ptr<Pickable>& pickable,
                                       BatcherId batcher_id) {
  PickingId id = GetOrCreatePickableId(pickable, batcher_id);
  return ColorFromPickingID(id);
}

bool PickingManager::IsThisElementPicked(const Pickable* pickable) const {
  auto picked = GetPicked();
  return picked && picked.get() == pickable;
}

Color PickingManager::ColorFromPickingID(PickingId id) {
  auto color_values = absl::bit_cast<std::array<uint8_t, 4>>(id.ToPixelValue());
  return {color_values[0], color_values[1], color_values[2], color_values[3]};
}
