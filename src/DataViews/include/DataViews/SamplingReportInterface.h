// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_SAMPLING_REPORT_INTERFACE_H_
#define DATA_VIEWS_SAMPLING_REPORT_INTERFACE_H_

#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <vector>

#include "ClientData/CallstackType.h"
#include "DataViews/CallstackDataView.h"

namespace orbit_data_views {

class SamplingReportInterface {
 public:
  virtual ~SamplingReportInterface() = default;

  virtual void SetCallstackDataView(CallstackDataView* data_view) = 0;
  virtual void OnSelectAddresses(const absl::flat_hash_set<uint64_t>& addresses,
                                 orbit_client_data::ThreadID thread_id) = 0;
  [[nodiscard]] virtual const orbit_client_data::CallstackData& GetCallstackData() const = 0;
  [[nodiscard]] virtual std::optional<absl::flat_hash_set<uint64_t>> GetSelectedCallstackIds()
      const = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_SAMPLING_REPORT_INTERFACE_H_