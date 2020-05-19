//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PickingManager.h"

#include "OpenGl.h"

//-----------------------------------------------------------------------------
PickingID PickingManager::CreatePickableId(Pickable* a_Pickable) {
  absl::MutexLock lock(&mutex_);
  ++m_IdCounter;
  PickingID id = PickingID::Get(PickingID::PICKABLE, m_IdCounter);
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
  return m_IdPickableMap[id];
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
Color PickingManager::GetPickableColor(Pickable* pickable) {
  PickingID id = CreatePickableId(pickable);
  return ColorFromPickingID(id);
}

//-----------------------------------------------------------------------------
Color PickingManager::ColorFromPickingID(PickingID id) const {
  const Color* color = reinterpret_cast<const Color*>(&id);
  return *color;
}
