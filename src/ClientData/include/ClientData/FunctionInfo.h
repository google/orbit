// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FUNCTION_INFO_H_
#define CLIENT_DATA_FUNCTION_INFO_H_

#include <optional>
#include <string>
#include <utility>

#include "GrpcProtos/symbol.pb.h"

namespace orbit_client_data {

class ModuleData;
class ProcessData;

// This class is used on the client to represent a function, containing a name, module path and
// build ID, address and size.
class FunctionInfo {
 public:
  FunctionInfo() = delete;
  FunctionInfo(std::string pretty_name, std::string module_path, std::string module_build_id,
               uint64_t address, uint64_t size)
      : pretty_name_(std::move(pretty_name)),
        module_path_(std::move(module_path)),
        module_build_id_(std::move(module_build_id)),
        address_(address),
        size_(size) {}

  FunctionInfo(const orbit_grpc_protos::SymbolInfo& symbol_info, std::string module_path,
               std::string module_build_id)
      : pretty_name_{symbol_info.demangled_name()},
        module_path_{std::move(module_path)},
        module_build_id_{std::move(module_build_id)},
        address_{symbol_info.address()},
        size_{symbol_info.size()} {}

  [[nodiscard]] const std::string& pretty_name() const { return pretty_name_; }
  [[nodiscard]] const std::string& module_path() const { return module_path_; }
  [[nodiscard]] const std::string& module_build_id() const { return module_build_id_; }
  [[nodiscard]] uint64_t address() const { return address_; }
  [[nodiscard]] uint64_t size() const { return size_; }

  [[nodiscard]] std::string GetLoadedModuleName() const;
  [[nodiscard]] uint64_t GetHash() const;

  // Calculates and returns the absolute address of the function.
  [[nodiscard]] uint64_t Offset(const ModuleData& module) const;

  // This function should not be used, since it could return incomplete or invalid
  // result in the case when one module is mapped two or more times. Try to find another way
  // of solving the problem, for example by converting an absolute address to a module offset and
  // then operating on that.
  // TODO(b/191248550): Disassemble from file instead of doing it from process memory.
  [[nodiscard, deprecated]] std::optional<uint64_t> GetAbsoluteAddress(
      const ProcessData& process, const ModuleData& module) const;

  [[nodiscard]] bool IsFunctionSelectable() const;

  bool operator==(const FunctionInfo& rhs) const {
    // Compare functions by module path and address
    return module_path() == rhs.module_path() && module_build_id() == rhs.module_build_id() &&
           address() == rhs.address();
  }
  bool operator!=(const FunctionInfo& rhs) const { return !(*this == rhs); }

  template <typename H>
  friend H AbslHashValue(H h, const FunctionInfo& o) {
    return H::combine(std::move(h), o.address(), o.module_build_id(), o.module_path());
  }

 private:
  std::string pretty_name_;
  std::string module_path_;
  std::string module_build_id_;
  uint64_t address_;
  uint64_t size_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FUNCTION_INFO_H_
