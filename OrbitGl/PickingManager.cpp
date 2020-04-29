//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PickingManager.h"

#include "OpenGl.h"

//-----------------------------------------------------------------------------
PickingID PickingManager::CreatePickableId(Pickable* a_Pickable) {
  ++m_IdCounter;
  PickingID id = PickingID::Get(PickingID::PICKABLE, ++m_IdCounter);
  m_PickableIdMap[a_Pickable] = m_IdCounter;
  m_IdPickableMap[m_IdCounter] = a_Pickable;
  return id;
}

//-----------------------------------------------------------------------------
void PickingManager::ClearIds() {
  m_IdPickableMap.clear();
  m_PickableIdMap.clear();
}

//-----------------------------------------------------------------------------
void PickingManager::Pick(uint32_t a_Id, int a_X, int a_Y) {
  m_Picked = m_IdPickableMap[a_Id];
  if (m_Picked) {
    m_Picked->OnPick(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
void PickingManager::Release() {
  if (m_Picked) {
    m_Picked->OnRelease();
  }

  m_Picked = nullptr;
}

//-----------------------------------------------------------------------------
void PickingManager::Drag(int a_X, int a_Y) {
  if (m_Picked) {
    m_Picked->OnDrag(a_X, a_Y);
  }
}

//-----------------------------------------------------------------------------
void PickingManager::SetPickingColor(PickingID a_ID) {
  glColor4ubv(reinterpret_cast<const uint8_t*>(&a_ID));
}

//-----------------------------------------------------------------------------
Color PickingManager::ColorFromPickingID(PickingID id) const {
  static_assert(sizeof(PickingID) == sizeof(Color),
                "PickingID should be same size as Color");
  const Color* color = reinterpret_cast<const Color*>(&id);
  return *color;
}
