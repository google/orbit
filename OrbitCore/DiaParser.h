/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <string>

#include "BaseTypes.h"
#include "dia2dump.h"

struct IDiaSymbol;
struct IDiaSourceFile;
struct IDiaEnumLineNumbers;
struct IDiaSectionContrib;
struct IDiaEnumDebugStreamData;
struct IDiaFrameData;
struct IDiaPropertyStorage;

//-----------------------------------------------------------------------------
struct DiaParser {
  static std::wstring GetSymTag(DWORD dwSymTag);
  static std::wstring GetLocation(IDiaSymbol*);
  static std::wstring GetSymbolType(IDiaSymbol*);
  static std::wstring GetName(IDiaSymbol*);

  std::string GetBasicType(uint32_t a_BaseType);
  ULONGLONG GetSize(IDiaSymbol*);
  DWORD GetSymbolID(IDiaSymbol*);
  DWORD GetTypeID(IDiaSymbol*);
  void GetData(IDiaSymbol*, class Type*);
  std::wstring GetData(IDiaSymbol*);

  void PrintPublicSymbol(IDiaSymbol*);
  void PrintGlobalSymbol(IDiaSymbol*);
  void OrbitAddGlobalSymbol(IDiaSymbol*);
  void PrintSymbol(IDiaSymbol*, DWORD);
  void PrintSymTag(DWORD);
  void PrintName(IDiaSymbol*);
  void PrintUndName(IDiaSymbol*);
  void PrintThunk(IDiaSymbol*);
  void PrintCompilandDetails(IDiaSymbol*);
  void PrintCompilandEnv(IDiaSymbol*);
  void PrintLocation(IDiaSymbol*);
  void PrintConst(IDiaSymbol*);
  void PrintUDT(IDiaSymbol*);
  void PrintSymbolType(IDiaSymbol*);
  void PrintSymbolTypeNoPrefix(IDiaSymbol*);
  void PrintType(IDiaSymbol*);
  void PrintBound(IDiaSymbol*);
  void PrintData(IDiaSymbol*);
  void PrintUdtKind(IDiaSymbol*);
  void PrintTypeInDetail(IDiaSymbol*, DWORD);
  void PrintFunctionType(IDiaSymbol*);
  void PrintSourceFile(IDiaSourceFile*);
  void PrintLines(IDiaSession*, IDiaSymbol*);
  void PrintLines(IDiaEnumLineNumbers*);
  void PrintSource(IDiaSourceFile*);
  void PrintSecContribs(IDiaSectionContrib*);
  void PrintStreamData(IDiaEnumDebugStreamData*);
  void PrintFrameData(IDiaFrameData*);
  void PrintPropertyStorage(IDiaPropertyStorage*);
  void PrintCallSiteInfo(IDiaSymbol*);
  void PrintHeapAllocSite(IDiaSymbol*);
  void PrintCoffGroup(IDiaSymbol*);
  void TypeLogSymTag(DWORD dwSymTag);
  void PrintNameTypeLog(IDiaSymbol*);

  void PrintClassHierarchy(IDiaSymbol*, DWORD, IDiaSymbol* a_Parent = nullptr);

  void GetTypeInformation(class Type* a_Type, DWORD a_TagType);
  void GetTypeInformation(class Type* a_Type, IDiaSymbol* pSymbol,
                          DWORD a_TagType, DWORD dwIndent);

  template <typename... Args>
  inline void LOGF(_In_z_ _Printf_format_string_ const wchar_t* const _Format,
                   Args&&... args) {
    m_Log += Format(_Format, std::forward<Args>(args)...);
  }

  std::wstring m_Log;
};
