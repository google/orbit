//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include "absl/strings/str_format.h"
#include "Utils.h"
#include "BaseTypes.h"

//-----------------------------------------------------------------------------
class Disassembler {
 public:
  void Disassemble(const unsigned char* a_MachineCode, size_t a_Size,
                   DWORD64 a_Address, bool a_Is64Bit);
  const std::wstring& GetResult() { return m_String; }

  void LogHex(const unsigned char* str, size_t len);

 public:
  std::wstring m_String;
};
