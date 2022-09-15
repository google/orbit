// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "BaselineAndComparisonHelper.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>

#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarData/MizarDataProvider.h"

using ::orbit_mizar_base::AbsoluteAddress;

namespace orbit_mizar_data {

using SFID = ::orbit_mizar_base::SFID;

[[nodiscard]] static absl::flat_hash_set<std::string> FunctionNamesSet(
    absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> map) {
  absl::flat_hash_set<std::string> result;
  std::transform(std::begin(map), std::end(map), std::inserter(result, std::begin(result)),
                 [](const std::pair<AbsoluteAddress, FunctionSymbol>& pair) {
                   return pair.second.function_name;
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

[[nodiscard]] AddressToIdAndIdToName AssignSampledFunctionIds(
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& baseline_address_to_symbol,
    const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& comparison_address_to_symbol) {
  absl::flat_hash_set<std::string> baseline_names = FunctionNamesSet(baseline_address_to_symbol);
  absl::flat_hash_set<std::string> comparison_names =
      FunctionNamesSet(comparison_address_to_symbol);

  absl::flat_hash_map<std::string, SFID> name_to_sfid;
  absl::flat_hash_map<SFID, std::string> sfid_to_name;

  SFID next_sfid_value{1};
  for (const std::string& name : baseline_names) {
    if (comparison_names.contains(name) && !name_to_sfid.contains(name)) {
      name_to_sfid.try_emplace(name, next_sfid_value);
      sfid_to_name.try_emplace(next_sfid_value, name);
      ++next_sfid_value;
    }
  }

  absl::flat_hash_map<AbsoluteAddress, SFID> baseline_address_to_sfid =
      AddressToSFID(baseline_address_to_symbol, name_to_sfid);
  absl::flat_hash_map<AbsoluteAddress, SFID> comparison_address_to_sfid =
      AddressToSFID(comparison_address_to_symbol, name_to_sfid);

  return {std::move(baseline_address_to_sfid), std::move(comparison_address_to_sfid),
          std::move(sfid_to_name)};
}

}  // namespace orbit_mizar_data