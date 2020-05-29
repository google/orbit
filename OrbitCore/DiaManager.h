// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#ifdef _WIN32

#include <string>

struct IDiaDataSource;
struct IDiaSession;
struct IDiaSymbol;
struct IDiaEnumSymbols;
struct IDiaSourceFile;
struct IDiaEnumSourceFiles;
struct IDiaEnumLineNumbers;
struct IDiaLineNumber;
struct IUnknown;

//-----------------------------------------------------------------------------
void OrbitDiaReleasePtr(IUnknown* a_Symbol);
void OrbitSmartPtrCreated();
void OrbitSmartPtrDestroyed();

//-----------------------------------------------------------------------------
template <typename T>
struct OrbitDiaSmartPtr {
  OrbitDiaSmartPtr() : OrbitDiaSmartPtr(nullptr) { OrbitSmartPtrCreated(); }
  OrbitDiaSmartPtr(T* a_Symbol) : m_Symbol(a_Symbol) { OrbitSmartPtrCreated(); }
  ~OrbitDiaSmartPtr() {
    if (m_Symbol) OrbitDiaReleasePtr((IUnknown*)m_Symbol);
    OrbitSmartPtrDestroyed();
  }
  void Release() {
    OrbitDiaReleasePtr(m_Symbol);
    m_Symbol = nullptr;
    OrbitSmartPtrDestroyed();
  }
  T* operator->() { return m_Symbol; }
  T* m_Symbol;
};

//-----------------------------------------------------------------------------
typedef OrbitDiaSmartPtr<IDiaSymbol> OrbitDiaSymbol;
typedef OrbitDiaSmartPtr<IDiaEnumSymbols> OrbitDiaEnumSymbols;
typedef OrbitDiaSmartPtr<IDiaSourceFile> OrbitDiaSourceFile;
typedef OrbitDiaSmartPtr<IDiaEnumSourceFiles> OrbitDiaEnumSourceFiles;
typedef OrbitDiaSmartPtr<IDiaEnumLineNumbers> OrbitDiaEnumLineNumbers;
typedef OrbitDiaSmartPtr<IDiaLineNumber> OrbitDiaLineNumber;

//-----------------------------------------------------------------------------
class DiaManager {
 public:
  DiaManager();
  ~DiaManager();

  bool LoadDataFromPdb(const wchar_t* a_FileName,
                       IDiaDataSource** a_DiaDataSource,
                       IDiaSession** a_Session, IDiaSymbol** a_GlobalSymbol);
  static void InitMsDiaDll();
};

#endif