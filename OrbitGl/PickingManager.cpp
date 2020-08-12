// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PickingManager.h"

#include "OpenGl.h"
#include "OrbitBase/Logging.h"

PickingID PickingManager::CreatePickableId(std::weak_ptr<Pickable> a_Pickable,
                                           PickingID::BatcherId batcher_id) {
  absl::MutexLock lock(&mutex_);
  ++id_counter_;
  PickingID id =
      PickingID::Get(PickingID::Type::kPickable, id_counter_, batcher_id);
  id_pickable_map_[id_counter_] = a_Pickable;
  return id;
}

void PickingManager::Reset() {
  absl::MutexLock lock(&mutex_);
  id_pickable_map_.clear();
  id_counter_ = 0;
}

std::weak_ptr<Pickable> PickingManager::GetPickableFromId(uint32_t id) const {
  absl::MutexLock lock(&mutex_);
  auto it = id_pickable_map_.find(id);
  if (it == id_pickable_map_.end()) {
    return std::weak_ptr<Pickable>();
  }
  return it->second;
}

std::weak_ptr<Pickable> PickingManager::GetPicked() const {
  absl::MutexLock lock(&mutex_);
  return currently_picked_;
}

void PickingManager::Pick(uint32_t id, int x, int y) {
  auto picked = GetPickableFromId(id).lock();
  if (picked) {
    picked->OnPick(x, y);
  }

  absl::MutexLock lock(&mutex_);
  currently_picked_ = picked;
}

void PickingManager::Release() {
  auto picked = GetPicked().lock();
  if (picked != nullptr) {
    picked->OnRelease();
    absl::MutexLock lock(&mutex_);
    currently_picked_.reset();
  }
}

void PickingManager::Drag(int x, int y) {
  auto picked = GetPicked().lock();
  if (picked && picked->Draggable()) {
    picked->OnDrag(x, y);
  }
}

bool PickingManager::IsDragging() const {
  absl::MutexLock lock(&mutex_);
  auto picked = currently_picked_.lock();
  return picked && picked->Draggable();
}

Color PickingManager::GetPickableColor(std::weak_ptr<Pickable> pickable,
                                       PickingID::BatcherId batcher_id) {
  PickingID id = CreatePickableId(pickable, batcher_id);
  return ColorFromPickingID(id);
}

bool PickingManager::IsThisElementPicked(const Pickable* pickable) const {
  auto picked = GetPicked().lock();
  return picked && picked.get() == pickable;
}

Color PickingManager::ColorFromPickingID(PickingID id) const {
  static_assert(sizeof(PickingID) == 4 * sizeof(uint8_t),
                "PickingID should be same size as 4 * uint8_t");
  std::array<uint8_t, 4> color_values;
  std::memcpy(&color_values[0], &id, sizeof(PickingID));
  return Color(color_values[0], color_values[1], color_values[2],
               color_values[3]);
}
