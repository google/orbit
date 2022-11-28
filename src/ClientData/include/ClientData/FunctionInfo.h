// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FUNCTION_INFO_H_
#define CLIENT_DATA_FUNCTION_INFO_H_

#include <stdint.h>

#include <optional>
#include <string>
#include <utility>

#include "GrpcProtos/symbol.pb.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_client_data {

class ModuleData;
class ProcessData;

// This class is used on the client to represent a function, containing a name, module path and
// build ID, address, size and whether the function is hotpatchable.
class FunctionInfo {
 public:
  FunctionInfo() = delete;
  FunctionInfo(std::string module_path, std::string module_build_id, uint64_t address,
               uint64_t size, std::string pretty_name, bool is_hotpatchable)
      : module_path_(std::move(module_path)),
        module_build_id_(std::move(module_build_id)),
        address_(address),
        size_(size),
        pretty_name_(std::move(pretty_name)),
        is_hotpatchable_(is_hotpatchable) {}

  FunctionInfo(const orbit_grpc_protos::SymbolInfo& symbol_info, std::string module_path,
               std::string module_build_id)
      : module_path_{std::move(module_path)},
        module_build_id_{std::move(module_build_id)},
        address_{symbol_info.address()},
        size_{symbol_info.size()},
        pretty_name_{symbol_info.demangled_name()},
        is_hotpatchable_(symbol_info.is_hotpatchable()) {}

  [[nodiscard]] const std::string& module_path() const { return module_path_; }
  [[nodiscard]] const std::string& module_build_id() const { return module_build_id_; }
  [[nodiscard]] orbit_symbol_provider::ModuleIdentifier module_id() const {
    return orbit_symbol_provider::ModuleIdentifier{module_path(), module_build_id()};
  }
  // The virtual address as specified in the object file.
  [[nodiscard]] uint64_t address() const { return address_; }
  [[nodiscard]] uint64_t size() const { return size_; }
  [[nodiscard]] const std::string& pretty_name() const { return pretty_name_; }
  [[nodiscard]] bool IsHotpatchable() const { return is_hotpatchable_; }

  [[nodiscard]] uint64_t GetPrettyNameHash() const;

  // Calculates and returns the offset of the function in the module file. For ELF files, this is
  // simply "address - load bias", by definition of load bias. For PEs, this is more complicated, as
  // it requires to find the section whose virtual address range contains the address of the
  // function.
  [[nodiscard]] uint64_t ComputeFileOffset(const ModuleData& module) const;

  // This function should not be used, since it could return incomplete or invalid
  // result in the case when one module is mapped two or more times. Try to find another way
  // of solving the problem, for example by converting an absolute address to a module offset and
  // then operating on that.
  // TODO(b/191248550): Disassemble from file instead of doing it from process memory.
  // Please also remove the forward declaration of ProcessData and ModuleData when removing this.
  [[nodiscard, deprecated]] std::optional<uint64_t> GetAbsoluteAddress(
      const ProcessData& process, const ModuleData& module) const;

  [[nodiscard]] bool IsFunctionSelectable() const;

  void SetAddress(uint64_t address) { address_ = address; }

  [[nodiscard]] friend bool operator==(const FunctionInfo& lhs, const FunctionInfo& rhs) {
    // Compare functions by module path, build_id and address. Explicitly ignore function name and
    // size.
    // TODO(b/227562690): We should also consider name and size.
    return lhs.module_path() == rhs.module_path() &&
           lhs.module_build_id() == rhs.module_build_id() && lhs.address() == rhs.address();
  }
  [[nodiscard]] friend bool operator!=(const FunctionInfo& lhs, const FunctionInfo& rhs) {
    return !(lhs == rhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, const FunctionInfo& o) {
    // Hashes by module path, build_id and address. Explicitly ignore function name and
    // size.
    // TODO(b/227562690): We should also consider name and size.
    return H::combine(std::move(h), o.address(), o.module_build_id(), o.module_path());
  }

 private:
  std::string module_path_;
  std::string module_build_id_;
  uint64_t address_;
  uint64_t size_;
  std::string pretty_name_;
  bool is_hotpatchable_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FUNCTION_INFO_H_
