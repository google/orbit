// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PickingManager.h"

#include "OpenGl.h"
#include "OrbitBase/Logging.h"

//-----------------------------------------------------------------------------
PickingID PickingManager::CreatePickableId(Pickable* a_Pickable, PickingID::BatcherId batcher_id) {
  absl::MutexLock lock(&mutex_);
  ++m_IdCounter;
  PickingID id = PickingID::Get(PickingID::PICKABLE, m_IdCounter, batcher_id);
  m_PickableIdMap[a_Pickable] = m_IdCounter;
  m_IdPickableMap[m_IdCounter] = a_Pickable;
  return id;
}

//-----------------------------------------------------------------------------
void PickingManager::Reset() {
  absl::MutexLock lock(&mutex_);
  m_IdPickableMap.clear();
  m_PickableIdMap.clear();
  m_IdCounter = 0;
}

//-----------------------------------------------------------------------------
Pickable* PickingManager::GetPickableFromId(uint32_t id) {
  absl::MutexLock lock(&mutex_);
  auto it = m_IdPickableMap.find(id);
  if (it == m_IdPickableMap.end()) {
    return nullptr;
  }
  return it->second;
}

//-----------------------------------------------------------------------------
Pickable* PickingManager::GetPicked() {
  absl::MutexLock lock(&mutex_);
  return m_Picked;
}

//-----------------------------------------------------------------------------
void PickingManager::Pick(uint32_t a_Id, int a_X, int a_Y) {
  Pickable* picked = GetPickableFromId(a_Id);
  if (picked) {
    picked->OnPick(a_X, a_Y);
  }

  absl::MutexLock lock(&mutex_);
  m_Picked = picked;
}

//-----------------------------------------------------------------------------
void PickingManager::Release() {
  Pickable* picked = GetPicked();
  if (picked != nullptr) {
    picked->OnRelease();
    absl::MutexLock lock(&mutex_);
    m_Picked = nullptr;
  }
}

//-----------------------------------------------------------------------------
void PickingManager::Drag(int a_X, int a_Y) {
  Pickable* picked = GetPicked();
  if (picked) {
    picked->OnDrag(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
bool PickingManager::IsDragging() const {
  absl::MutexLock lock(&mutex_);
  return m_Picked && m_Picked->Draggable();
}

//-----------------------------------------------------------------------------
Color PickingManager::GetPickableColor(Pickable* pickable, PickingID::BatcherId batcher_id) {
  PickingID id = CreatePickableId(pickable, batcher_id);
  return ColorFromPickingID(id);
}

//-----------------------------------------------------------------------------
Color PickingManager::ColorFromPickingID(PickingID id) const {
  static_assert(sizeof(PickingID) == sizeof(Color),
                "PickingID should be same size as Color");
  const Color* color = reinterpret_cast<const Color*>(&id);
  return *color;
}
