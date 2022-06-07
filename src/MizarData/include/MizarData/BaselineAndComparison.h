// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_AND_COMPARISON_H_
#define MIZAR_DATA_BASELINE_AND_COMPARISON_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <utility>

#include "ClientData/CaptureData.h"
#include "MizarData/MizarData.h"

namespace orbit_mizar_data {

struct MizarDataWithSampledFunctionId {
  MizarDataWithSampledFunctionId(
      std::unique_ptr<MizarDataProvider> data,
      absl::flat_hash_map<uint64_t, uint64_t> address_to_sampled_function_id)
      : data(std::move(data)),
        address_to_sampled_function_id(std::move(address_to_sampled_function_id)) {}

  std::unique_ptr<MizarDataProvider> data;
  absl::flat_hash_map<uint64_t, uint64_t> address_to_sampled_function_id;
};

// The class owns the data from two capture files via owning two instances of
// `MizarDataWithSampledFunctionId`. Also owns the map from sampled function ids to the
// corresponding function names.
class BaselineAndComparison {
 public:
  BaselineAndComparison(MizarDataWithSampledFunctionId baseline,
                        MizarDataWithSampledFunctionId comparison,
                        absl::flat_hash_map<uint64_t, std::string> sampled_function_id_to_name)
      : baseline_(std::move(baseline)),
        comparison_(std::move(comparison)),
        sampled_function_id_to_name_(std::move(sampled_function_id_to_name)) {}

  [[nodiscard]] const absl::flat_hash_map<uint64_t, std::string>& sampled_function_id_to_name()
      const {
    return sampled_function_id_to_name_;
  }

 private:
  MizarDataWithSampledFunctionId baseline_;
  MizarDataWithSampledFunctionId comparison_;
  absl::flat_hash_map<uint64_t, std::string> sampled_function_id_to_name_;
};

orbit_mizar_data::BaselineAndComparison CreateBaselineAndComparison(
    std::unique_ptr<MizarDataProvider> baseline, std::unique_ptr<MizarDataProvider> comparison);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_AND_COMPARISON_H_