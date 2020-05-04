// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"
#include "SerializationMacros.h"

struct Params {
  Params();
  void Load();
  void Save();

  void AddToPdbHistory(const std::string& a_PdbName);

 public:
  bool m_LoadTypeInfo;
  bool m_SendCallStacks;
  bool m_TrackContextSwitches;
  bool m_TrackSamplingEvents;
  bool m_UnrealSupport;
  bool m_UnitySupport;
  bool m_StartPaused;
  bool m_AllowUnsafeHooking;
  bool m_HookOutputDebugString;
  bool m_FindFileAndLineInfo;
  bool m_BpftraceCallstacks;
  bool m_SystemWideScheduling;
  bool m_UseBpftrace;
  bool m_UploadDumpsToServer;
  int m_MaxNumTimers;
  float m_FontSize;
  int m_Port;
  uint64_t m_NumBytesAssembly;
  std::string m_DiffExe;
  std::string m_DiffArgs;
  std::vector<std::string> m_PdbHistory;
  std::unordered_map<std::string, std::string> m_CachedPdbsMap;
  std::string m_ProcessPath;
  std::string m_Arguments;
  std::string m_WorkingDirectory;
  std::string m_ProcessFilter;

  ORBIT_SERIALIZABLE;
};

extern Params GParams;
