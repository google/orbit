//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitUnreal.h"

#include "OrbitType.h"

OrbitUnreal GOrbitUnreal;

//-----------------------------------------------------------------------------
void OrbitUnreal::OnTypeAdded(Type* a_Type) {
  if (a_Type->m_Name == L"FNameEntry") {
    m_FnameEntryType = a_Type;
  } else if (a_Type->m_Name == L"UObject") {
    m_UObjectType = a_Type;
  }
}

//-----------------------------------------------------------------------------
void OrbitUnreal::OnFunctionAdded(Function* a_Function) {
  if (a_Function->m_PrettyName == "FName::GetDisplayNameEntry") {
    m_GetDisplayNameEntryFunc = a_Function;
  }
}

//-----------------------------------------------------------------------------
bool OrbitUnreal::HasFnameInfo() {
  if (m_FnameEntryType && m_GetDisplayNameEntryFunc && m_UObjectType) {
    return GenerateUnrealInfo();
  }

  return false;
}

//-----------------------------------------------------------------------------
bool OrbitUnreal::GenerateUnrealInfo() {
#ifdef WIN32
  OrbitUnrealInfo info;
  info.m_GetDisplayNameEntryAddress =
      GOrbitUnreal.m_GetDisplayNameEntryFunc->GetVirtualAddress();
  info.m_EntryIndexOffset = m_FnameEntryType->GetOffset(L"Index");
  info.m_UobjectNameOffset = m_UObjectType->GetOffset(L"Name");
  info.m_EntryNameOffset = m_FnameEntryType->GetOffset(L"AnsiName");
  if (info.m_EntryNameOffset == -1) {
    info.m_EntryNameOffset = m_FnameEntryType->GetOffset(L"WideName");
  }

  if (info.m_EntryNameOffset == -1 || info.m_EntryIndexOffset == -1 ||
      info.m_UobjectNameOffset == -1) {
    return false;
  }

  m_UnrealInfo = info;
#endif
  return true;
}

//-----------------------------------------------------------------------------
void OrbitUnreal::NewSession() { m_ObjectNames.clear(); }

//-----------------------------------------------------------------------------
void OrbitUnreal::Clear() {
  m_ObjectNames.clear();
  m_UObjectType = nullptr;
  m_FnameEntryType = nullptr;
  m_GetDisplayNameEntryFunc = nullptr;
}

//-----------------------------------------------------------------------------
const OrbitUnrealInfo& OrbitUnreal::GetUnrealInfo() { return m_UnrealInfo; }