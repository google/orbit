// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <string>
#include <tuple>
#include <utility>

#include "DummyFunctionSymbolToKey.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/SampledFunctionId.h"

namespace orbit_mizar_data {

struct AddressToIdAndIdToSymbol {
  absl::flat_hash_map<orbit_mizar_base::AbsoluteAddress, orbit_mizar_base::SFID>
      baseline_address_to_sfid;
  absl::flat_hash_map<orbit_mizar_base::AbsoluteAddress, orbit_mizar_base::SFID>
      comparison_address_to_sfid;
  absl::flat_hash_map<orbit_mizar_base::SFID,
                      orbit_mizar_base::BaselineAndComparisonFunctionSymbols>
      sfid_to_symbols;
};

// `FunctionSymbolToKey` is default-constructible and defines `Key GetKey(const FunctionSymbol&)`
// method. `Key` must be absl-hashable and define `operator==`. The functions with `FunctionSymbols`
// mapped to equal `Key` will be assigned the same `SFID`.
template <typename FunctionSymbolToKey, typename Key>
class BaselineAndComparisonHelperTmpl {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using AbsoluteAddress = ::orbit_mizar_base::AbsoluteAddress;
  using BaselineAndComparisonFunctionSymbols =
      ::orbit_mizar_base::BaselineAndComparisonFunctionSymbols;
  using FunctionSymbol = ::orbit_mizar_base::FunctionSymbol;
  using SFID = ::orbit_mizar_base::SFID;

 public:
  [[nodiscard]] AddressToIdAndIdToSymbol AssignSampledFunctionIds(
      const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& baseline_address_to_symbol,
      const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& comparison_address_to_symbol)
      const {
    // TODO(b/248266436) the code needs to be commented
    absl::flat_hash_map<Key, FunctionSymbol> comparison_key_to_symbol =
        KeyToSymbol(comparison_address_to_symbol);

    absl::flat_hash_map<Key, SFID> key_to_sfid;
    absl::flat_hash_map<SFID, BaselineAndComparisonFunctionSymbols> sfid_to_symbols;

    SFID next_sfid_value{1};
    for (const auto& [unused_address, baseline_function_symbol] : baseline_address_to_symbol) {
      Key key = function_symbol_to_key_.GetKey(baseline_function_symbol);
      if (const auto comparison_key_to_symbol_it = comparison_key_to_symbol.find(key);
          comparison_key_to_symbol_it != comparison_key_to_symbol.end() &&
          !key_to_sfid.contains(key)) {
        key_to_sfid.try_emplace(std::move(key), next_sfid_value);

        BaselineAndComparisonFunctionSymbols symbols{
            Baseline<FunctionSymbol>(baseline_function_symbol),
            Comparison<FunctionSymbol>(comparison_key_to_symbol_it->second)};

        sfid_to_symbols.try_emplace(next_sfid_value, std::move(symbols));
        ++next_sfid_value;
      }
    }

    absl::flat_hash_map<AbsoluteAddress, SFID> baseline_address_to_sfid =
        AddressToSFID(baseline_address_to_symbol, key_to_sfid);
    absl::flat_hash_map<AbsoluteAddress, SFID> comparison_address_to_sfid =
        AddressToSFID(comparison_address_to_symbol, key_to_sfid);

    return {std::move(baseline_address_to_sfid), std::move(comparison_address_to_sfid),
            std::move(sfid_to_symbols)};
  }

 private:
  [[nodiscard]] absl::flat_hash_map<Key, FunctionSymbol> KeyToSymbol(
      absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> map) const {
    absl::flat_hash_map<Key, FunctionSymbol> result;
    absl::c_transform(map, std::inserter(result, std::begin(result)),
                      [this](const std::pair<AbsoluteAddress, FunctionSymbol>& pair) {
                        const FunctionSymbol& symbol = pair.second;
                        const Key key = function_symbol_to_key_.GetKey(symbol);
                        return std::make_pair(key, symbol);
                      });
    return result;
  }

  [[nodiscard]] absl::flat_hash_map<AbsoluteAddress, SFID> AddressToSFID(
      const absl::flat_hash_map<AbsoluteAddress, FunctionSymbol>& address_to_symbol,
      const absl::flat_hash_map<Key, SFID>& key_to_sfid) const {
    absl::flat_hash_map<AbsoluteAddress, SFID> address_to_sfid;
    for (const auto& [address, symbol] : address_to_symbol) {
      const Key key = function_symbol_to_key_.GetKey(symbol);
      if (const auto it = key_to_sfid.find(key); it != key_to_sfid.end()) {
        address_to_sfid.try_emplace(address, it->second);
      }
    }
    return address_to_sfid;
  }

  FunctionSymbolToKey function_symbol_to_key_;
};

// The instantiation used in production
using BaselineAndComparisonHelper =
    BaselineAndComparisonHelperTmpl<D3D11DummyFunctionSymbolToKey, std::string>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_HELPER_H_
