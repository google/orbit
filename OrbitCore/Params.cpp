// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Params.h"

#include <algorithm>
#include <fstream>

#include "Core.h"
#include "CoreApp.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include "absl/strings/str_format.h"

Params GParams;

//-----------------------------------------------------------------------------
Params::Params()
    : m_LoadTypeInfo(true),
      m_SendCallStacks(true),
      m_TrackContextSwitches(true),
      m_TrackSamplingEvents(true),
      m_UnrealSupport(true),
      m_UnitySupport(true),
      m_StartPaused(true),
      m_AllowUnsafeHooking(false),
      m_HookOutputDebugString(false),
      m_FindFileAndLineInfo(true),
      m_SystemWideScheduling(true),
      m_UploadDumpsToServer(false),
      m_MaxNumTimers(1000000),
      m_FontSize(14.f),
      m_Port(44766),
      m_NumBytesAssembly(1024) {}

ORBIT_SERIALIZE(Params, 19) {
  ORBIT_NVP_VAL(0, m_LoadTypeInfo);
  ORBIT_NVP_VAL(0, m_SendCallStacks);
  ORBIT_NVP_VAL(0, m_MaxNumTimers);
  ORBIT_NVP_VAL(0, m_FontSize);
  ORBIT_NVP_VAL(0, m_PdbHistory);
  ORBIT_NVP_VAL(1, m_TrackContextSwitches);
  ORBIT_NVP_VAL(3, m_UnrealSupport);
  ORBIT_NVP_VAL(3, m_UnitySupport);
  ORBIT_NVP_VAL(3, m_StartPaused);
  ORBIT_NVP_VAL(4, m_AllowUnsafeHooking);
  ORBIT_NVP_VAL(5, m_Port);
  ORBIT_NVP_VAL(6, m_TrackSamplingEvents);
  ORBIT_NVP_VAL(8, m_NumBytesAssembly);
  ORBIT_NVP_VAL(9, m_HookOutputDebugString);
  ORBIT_NVP_VAL(10, m_ProcessPath);
  ORBIT_NVP_VAL(10, m_Arguments);
  ORBIT_NVP_VAL(10, m_WorkingDirectory);
  ORBIT_NVP_VAL(11, m_FindFileAndLineInfo);
  ORBIT_NVP_VAL(13, m_ProcessFilter);
  ORBIT_NVP_VAL(15, m_SystemWideScheduling);
  ORBIT_NVP_VAL(17, m_UploadDumpsToServer);
}

//-----------------------------------------------------------------------------
void Params::Save() {
  GCoreApp->SendToUiNow("UpdateProcessParams");

  std::string filename = Path::GetParamsFileName();
  std::ofstream file(filename);
  if (file.fail()) {
    ERROR("Saving Params in \"%s\"", filename);
    return;
  }

  try {
    cereal::XMLOutputArchive archive(file);
    archive(cereal::make_nvp("Params", *this));
  } catch (cereal::Exception& e) {
    ERROR("Saving Params in \"%s\": %s", filename, e.what());
  }
}

//-----------------------------------------------------------------------------
void Params::Load() {
  std::string filename = Path::GetParamsFileName();
  std::ifstream file(filename);
  if (file.fail()) {
    ERROR("Loading Params from \"%s\"", filename);
    // Try creating the file with default values, in case it doesn't exist.
    Save();
    return;
  }

  try {
    cereal::XMLInputArchive archive(file);
    archive(*this);
  } catch (cereal::Exception& e) {
    ERROR("Loading Params from \"%s\": %s", filename, e.what());
    // Try overwriting the file with default values, in case it's malformed.
    Save();
  }
}

//-----------------------------------------------------------------------------
void Params::AddToPdbHistory(const std::string& a_PdbName) {
  m_PdbHistory.push_back(a_PdbName);
  auto it = std::unique(m_PdbHistory.begin(), m_PdbHistory.end());
  m_PdbHistory.resize(std::distance(m_PdbHistory.begin(), it));
  Save();
}
