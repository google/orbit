//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#include "Batcher.h"

#include "Core.h"

//-----------------------------------------------------------------------------
TextBox* Batcher::GetTextBox(PickingID a_ID) {
  if (a_ID.m_Type == PickingID::BOX) {
    if (void** textBoxPtr = m_BoxBuffer.m_UserData.SlowAt(a_ID.m_Id)) {
      return (TextBox*)*textBoxPtr;
    }
  } else if (a_ID.m_Type == PickingID::LINE) {
    if (void** textBoxPtr = m_LineBuffer.m_UserData.SlowAt(a_ID.m_Id)) {
      return (TextBox*)*textBoxPtr;
    }
  }

  return nullptr;
}