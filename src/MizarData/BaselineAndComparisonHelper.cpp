// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BaselineAndComparisonHelper.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>

#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarData/MizarDataProvider.h"

using ::orbit_mizar_base::AbsoluteAddress;

namespace orbit_mizar_data {

template <typename T>
using Baseline = ::orbit_mizar_base::Baseline<T>;
template <typename T>
using Comparison = ::orbit_mizar_base::Comparison<T>;
using ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;
using ::orbit_mizar_base::FunctionSymbol;
using ::orbit_mizar_base::SFID;

[[nodiscard]] static absl::flat_hash_map<std::string, FunctionSymbol> FunctionNameToSymbol(
    absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> map) {
  absl::flat_hash_map<std::string, FunctionSymbol> result;
  absl::c_transform(map, std::inserter(result, std::begin(result)),
                    [](const std::pair<AbsoluteAddress, FunctionSymbol>& pair) {
                      const FunctionSymbol& symbol = pair.second;
                      return std::make_pair(symbol.function_name, symbol);
                    });
  return result;
}

static absl::flat_hash_map<AbsoluteAddress, SFID> AddressToSFID(
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& address_to_symbol,
    const absl::flat_hash_map<std::string, SFID>& name_to_sfid) {
  absl::flat_hash_map<AbsoluteAddress, SFID> address_to_sfid;
  for (const auto& [address, symbol] : address_to_symbol) {
    if (const auto it = name_to_sfid.find(symbol.function_name); it != name_to_sfid.end()) {
      address_to_sfid.try_emplace(address, it->second);
    }
  }
  return address_to_sfid;
}

[[nodiscard]] AddressToIdAndIdToSymbol AssignSampledFunctionIds(
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& baseline_address_to_symbol,
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& comparison_address_to_symbol) {
  absl::flat_hash_map<std::string, FunctionSymbol> comparison_names =
      FunctionNameToSymbol(comparison_address_to_symbol);

  absl::flat_hash_map<std::string, SFID> name_to_sfid;
  absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols> sfid_to_symbols;

  SFID next_sfid_value{1};
  for (const auto& [address, baseline_function_symbol] : baseline_address_to_symbol) {
    const std::string& name = baseline_function_symbol.function_name;
    if (const auto comparison_name_to_symbol_it = comparison_names.find(name);
        comparison_name_to_symbol_it != comparison_names.end() && !name_to_sfid.contains(name)) {
      name_to_sfid.try_emplace(name, next_sfid_value);

      BaselineAndComparisonFunctionSymbols symbols{
          Baseline<FunctionSymbol>(baseline_function_symbol),
          Comparison<FunctionSymbol>(comparison_name_to_symbol_it->second)};

      sfid_to_symbols.try_emplace(next_sfid_value, std::move(symbols));
      ++next_sfid_value;
    }
  }

  absl::flat_hash_map<AbsoluteAddress, SFID> baseline_address_to_sfid =
      AddressToSFID(baseline_address_to_symbol, name_to_sfid);
  absl::flat_hash_map<AbsoluteAddress, SFID> comparison_address_to_sfid =
      AddressToSFID(comparison_address_to_symbol, name_to_sfid);

  return {std::move(baseline_address_to_sfid), std::move(comparison_address_to_sfid),
          std::move(sfid_to_symbols)};
}

}  // namespace orbit_mizar_data