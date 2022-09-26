// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_
#define MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>

#include "MizarBase/FunctionSymbols.h"

namespace orbit_mizar_data {

// If a function comes from `mappableModules`, the key from `functionNameToKey` is returned if
// present. Defaulting to the function name as the key.
class DummyFunctionSymbolToKey {
  using FunctionSymbol = ::orbit_mizar_base::FunctionSymbol;

 public:
  DummyFunctionSymbolToKey(
      const absl::flat_hash_map<std::string, std::string>* function_name_to_key,
      const absl::flat_hash_set<std::string>* mappable_modules)
      : function_name_to_key_(function_name_to_key), mappable_modules_(mappable_modules) {}

  [[nodiscard]] std::string GetKey(const FunctionSymbol& symbol) const;

 private:
  const absl::flat_hash_map<std::string, std::string>* function_name_to_key_;
  const absl::flat_hash_set<std::string>* mappable_modules_;
};

class D3D11DummyFunctionSymbolToKey : public DummyFunctionSymbolToKey {
 public:
  D3D11DummyFunctionSymbolToKey()
      : DummyFunctionSymbolToKey(kDirectXToDxvkNames, kMappableModules) {}

 private:
  static const absl::flat_hash_map<std::string, std::string>* kDirectXToDxvkNames;
  static const absl::flat_hash_set<std::string>* kMappableModules;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_
