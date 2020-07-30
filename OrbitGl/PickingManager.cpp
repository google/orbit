// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PickingManager.h"

#include "OpenGl.h"
#include "OrbitBase/Logging.h"

//-----------------------------------------------------------------------------
PickingID PickingManager::CreatePickableId(std::weak_ptr<Pickable> a_Pickable,
                                           PickingID::BatcherId batcher_id) {
  absl::MutexLock lock(&mutex_);
  ++m_IdCounter;
  PickingID id = PickingID::Get(PickingID::PICKABLE, m_IdCounter, batcher_id);
  m_IdPickableMap[m_IdCounter] = a_Pickable;
  return id;
}

//-----------------------------------------------------------------------------
void PickingManager::Reset() {
  absl::MutexLock lock(&mutex_);
  m_IdPickableMap.clear();
  m_IdCounter = 0;
}

//-----------------------------------------------------------------------------
std::weak_ptr<Pickable> PickingManager::GetPickableFromId(uint32_t id) {
  absl::MutexLock lock(&mutex_);
  auto it = m_IdPickableMap.find(id);
  if (it == m_IdPickableMap.end()) {
    return std::weak_ptr<Pickable>();
  }
  return it->second;
}

//-----------------------------------------------------------------------------
std::weak_ptr<Pickable> PickingManager::GetPicked() {
  absl::MutexLock lock(&mutex_);
  return m_Picked;
}

//-----------------------------------------------------------------------------
void PickingManager::Pick(uint32_t a_Id, int a_X, int a_Y) {
  auto picked = GetPickableFromId(a_Id).lock();
  if (picked) {
    picked->OnPick(a_X, a_Y);
  }

  absl::MutexLock lock(&mutex_);
  m_Picked = picked;
}

//-----------------------------------------------------------------------------
void PickingManager::Release() {
  auto picked = GetPicked().lock();
  if (picked != nullptr) {
    picked->OnRelease();
    absl::MutexLock lock(&mutex_);
    m_Picked.reset();
  }
}

//-----------------------------------------------------------------------------
void PickingManager::Drag(int a_X, int a_Y) {
  auto picked = GetPicked().lock();
  if (picked && picked->Draggable()) {
    picked->OnDrag(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
bool PickingManager::IsDragging() {
  absl::MutexLock lock(&mutex_);
  auto picked = m_Picked.lock();
  return picked && picked->Draggable();
}

//-----------------------------------------------------------------------------
Color PickingManager::GetPickableColor(std::weak_ptr<Pickable> pickable,
                                       PickingID::BatcherId batcher_id) {
  PickingID id = CreatePickableId(pickable, batcher_id);
  return ColorFromPickingID(id);
}

bool PickingManager::IsThisElementPicked(Pickable* pickable) {
  auto picked = GetPicked().lock();
  return picked && picked.get() == pickable;
}

//-----------------------------------------------------------------------------
Color PickingManager::ColorFromPickingID(PickingID id) const {
  static_assert(sizeof(PickingID) == 4 * sizeof(uint8_t),
                "PickingID should be same size as 4 * uint8_t");
  std::array<uint8_t, 4> color_values;
  std::memcpy(&color_values[0], &id, sizeof(PickingID));
  return Color(color_values[0], color_values[1], color_values[2],
               color_values[3]);
}
