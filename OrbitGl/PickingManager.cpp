// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PickingManager.h"

#include "OpenGl.h"
#include "OrbitBase/Logging.h"

PickingId PickingManager::CreatePickableId(std::weak_ptr<Pickable> a_Pickable,
                                           BatcherId batcher_id) {
  absl::MutexLock lock(&mutex_);
  ++id_counter_;
  PickingId id =
      PickingId::Create(PickingType::kPickable, id_counter_, batcher_id);
  id_pickable_map_[id_counter_] = a_Pickable;
  return id;
}

void PickingManager::Reset() {
  absl::MutexLock lock(&mutex_);
  id_pickable_map_.clear();
  id_counter_ = 0;
}

std::weak_ptr<Pickable> PickingManager::GetPickableFromId(PickingId id) const {
  CHECK(id.type == PickingType::kPickable);

  absl::MutexLock lock(&mutex_);
  auto it = id_pickable_map_.find(id.element_id);
  if (it == id_pickable_map_.end()) {
    return std::weak_ptr<Pickable>();
  }
  return it->second;
}

std::weak_ptr<Pickable> PickingManager::GetPicked() const {
  absl::MutexLock lock(&mutex_);
  return currently_picked_;
}

void PickingManager::Pick(PickingId id, int x, int y) {
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
                                       BatcherId batcher_id) {
  PickingId id = CreatePickableId(pickable, batcher_id);
  return ColorFromPickingID(id);
}

bool PickingManager::IsThisElementPicked(const Pickable* pickable) const {
  auto picked = GetPicked().lock();
  return picked && picked.get() == pickable;
}

Color PickingManager::ColorFromPickingID(PickingId id) const {
  static_assert(sizeof(PickingId) == 4 * sizeof(uint8_t),
                "PickingId should be same size as 4 * uint8_t");
  std::array<uint8_t, 4> color_values;
  std::memcpy(&color_values[0], &id, sizeof(PickingId));
  return Color(color_values[0], color_values[1], color_values[2],
               color_values[3]);
}
