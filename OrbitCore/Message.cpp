// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Message.h"

#include "PrintVar.h"

//-----------------------------------------------------------------------------
uint32_t Message::GCaptureID;

//-----------------------------------------------------------------------------
void Message::Dump() {
  PRINT_VAR(offsetof(Message, m_Type));
  PRINT_VAR(offsetof(Message, m_Header));
  PRINT_VAR(offsetof(Message, m_Size));
  PRINT_VAR(offsetof(Message, m_CaptureID));
  PRINT_VAR(offsetof(Message, m_ThreadId));
  PRINT_VAR(offsetof(Message, m_Data));

  PRINT_VAR(sizeof(m_Type));
  PRINT_VAR(sizeof(m_Header));
  PRINT_VAR(sizeof(m_Size));
  PRINT_VAR(sizeof(m_CaptureID));
  PRINT_VAR(sizeof(m_ThreadId));
  PRINT_VAR(sizeof(m_Data));

  PRINT_VAR(sizeof(MessageGeneric));
  PRINT_VAR(sizeof(DataTransferHeader));
  PRINT_VAR(sizeof(ArgTrackingHeader));
  PRINT_VAR(sizeof(UnrealObjectHeader));
}
