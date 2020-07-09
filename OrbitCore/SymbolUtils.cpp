// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolUtils.h"

#include <psapi.h>
#include <tlhelp32.h>

#include "Capture.h"
#include "OrbitDbgHelp.h"
#include "OrbitModule.h"

//-----------------------------------------------------------------------------
bool SymUtils::GetLineInfo(DWORD64 a_Address, LineInfo& o_LineInfo) {
  std::shared_ptr<Process> process = Capture::GTargetProcess;

  if (process) {
    return process->LineInfoFromAddress(a_Address, o_LineInfo);
  }

  return false;
}

//-----------------------------------------------------------------------------
ScopeSymCleanup::ScopeSymCleanup(HANDLE a_Handle) : m_Handle(a_Handle) {}

//-----------------------------------------------------------------------------
ScopeSymCleanup::~ScopeSymCleanup() {
  if (!SymCleanup(m_Handle)) {
    ORBIT_ERROR;
  }
}
