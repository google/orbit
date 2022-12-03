// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <string_view>

#include "DummyFunctionSymbolToKey.h"
#include "MizarBase/FunctionSymbols.h"

using ::orbit_mizar_base::FunctionSymbol;

namespace orbit_mizar_data {

const std::string kMappedFunction = "foo";
const std::string kAnotherMappedFunction = "boo";
const std::string kNotMappedFunction = "bar";

static const auto* kNameToKey = new absl::flat_hash_map<std::string, std::string>{
    {kMappedFunction, "key1"}, {kAnotherMappedFunction, "key2"}};

const std::string kMappableModuleName = "mappable";
const std::string kNonMappableModuleName = "nonmappable";
static const auto* kMappableModules = new absl::flat_hash_set<std::string>{kMappableModuleName};

class SymbolToKey : public DummyFunctionSymbolToKey {
 public:
  SymbolToKey() : DummyFunctionSymbolToKey(kNameToKey, kMappableModules) {}
};

void ExpectCorrectKey(const SymbolToKey& symbol_to_key, std::string_view function_name,
                      std::string_view module_name) {
  FunctionSymbol symbol{std::string{function_name}, std::string{module_name}};
  const std::string key = symbol_to_key.GetKey(symbol);
  if (kMappableModules->contains(module_name) && kNameToKey->contains(function_name)) {
    EXPECT_EQ(key, kNameToKey->at(function_name));
  } else {
    EXPECT_EQ(key, function_name);
  }
}

TEST(DummyFunctionSymbolToKey, GetKey) {
  SymbolToKey symbol_to_key;

  for (const std::string& function :
       {kMappedFunction, kAnotherMappedFunction, kNonMappableModuleName}) {
    for (const std::string& module : {kMappableModuleName, kNonMappableModuleName}) {
      ExpectCorrectKey(symbol_to_key, function, module);
    }
  }
}

}  // namespace orbit_mizar_data