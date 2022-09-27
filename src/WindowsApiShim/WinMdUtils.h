// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_WIN_MD_UTILS_H_
#define ORBIT_WINDOWS_API_SHIM_WIN_MD_UTILS_H_

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <cppwin32/code_writers.h>
#include <cppwin32/file_writers.h>
#include <cppwin32/settings.h>
#include <cppwin32/text_writer.h>
#include <cppwin32/type_dependency_graph.h>
#include <cppwin32/type_writers.h>

#include <chrono>
#include <sstream>

#include "OrbitBase/Logging.h"

using cppwin32::method_signature;
using winmd::reader::cache;
using winmd::reader::CustomAttribute;
using winmd::reader::CustomAttributeSig;
using winmd::reader::database;
using winmd::reader::ElemSig;
using winmd::reader::FixedArgSig;
using winmd::reader::ImplMap;
using winmd::reader::MethodDef;
using winmd::reader::ModuleRef;
using winmd::reader::TypeDef;

namespace orbit_windows_api_shim {

bool IsX64(const MethodDef& method) {
  const CustomAttribute attr = winmd::reader::get_attribute(method, "Windows.Win32.Interop",
                                                            "SupportedArchitectureAttribute");
  if (attr) {
    CustomAttributeSig attr_sig = attr.Value();
    const FixedArgSig& fixed_arg = attr_sig.FixedArgs()[0];
    const ElemSig& elem_sig = std::get<ElemSig>(fixed_arg.value);
    const ElemSig::EnumValue& enum_value = std::get<ElemSig::EnumValue>(elem_sig.value);
    int32_t const arch_flags = std::get<int32_t>(enum_value.value);
    // None = 0, X86 = 1, X64 = 2, Arm64 = 4.
    constexpr const int32_t kX64 = 2;
    return ((arch_flags & kX64) != 0);
  }

  // Assume method is x64.
  return true;
}

template <typename T>
bool IsWindMdListEmpty(const T& list) {
  return list.first == list.second;
}

bool HasMethods(const std::vector<winmd::reader::TypeDef>& classes) {
  for (const auto& type : classes) {
    if (!IsWindMdListEmpty(type.MethodList())) return true;
  }
  return false;
}

const database* FindDatabaseFromName(const cache& cache, std::string_view database_name) {
  for (const database& db : cache.databases()) {
    std::filesystem::path path(db.path());
    if (path.filename() == database_name) {
      return &db;
    }
  }

  ORBIT_ERROR("Could not find database \"%s\"", database_name);
  return nullptr;
}

static std::string GetCurrentTimeFormatted() {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream string_stream;
  string_stream << std::ctime(&now);
  return string_stream.str();
}

}  // namespace orbit_windows_api_shim

#endif  // ORBIT_WINDOWS_API_SHIM_WIN_MD_UTILS_H_
