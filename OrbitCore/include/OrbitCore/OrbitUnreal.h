//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <unordered_map>

#include "Core.h"
#include "Message.h"

class Function;
class Type;

struct OrbitUnreal {
  void OnTypeAdded(Type* a_Type);
  void OnFunctionAdded(Function* a_Function);
  void NewSession();
  void Clear();

  bool HasFnameInfo();
  const OrbitUnrealInfo& GetUnrealInfo();
  std::unordered_map<DWORD64, std::wstring>& GetObjectNames() {
    return m_ObjectNames;
  }

 protected:
  bool GenerateUnrealInfo();

  Type* m_UObjectType;
  Type* m_FnameEntryType;
  Function* m_GetDisplayNameEntryFunc;
  std::unordered_map<DWORD64, std::wstring>
      m_ObjectNames;  // Don't access when capturing!
  OrbitUnrealInfo m_UnrealInfo;
};

extern OrbitUnreal GOrbitUnreal;