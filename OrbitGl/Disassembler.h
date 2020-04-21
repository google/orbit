//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include "BaseTypes.h"
#include "Utils.h"
#include "absl/strings/str_format.h"

class Disassembler {
 public:
  void Disassemble(const uint8_t* machine_code, size_t size, uint64_t address,
                   bool is_64bit);
  const std::string& GetResult() const { return result_; }

  void LOGF(const std::string& format) { result_ += format; }

  void LogHex(const uint8_t* str, size_t len);

 private:
  std::string result_;
};
