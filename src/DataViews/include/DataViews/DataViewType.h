// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_TYPE_H_
#define DATA_VIEWS_DATA_VIEW_TYPE_H_

namespace orbit_data_views {

enum class DataViewType {
  kInvalid,
  kFunctions,
  kLiveFunctions,
  kCallstack,
  kModules,
  kSampling,
  kPresets,
  kTracepoints,
  kAll,
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_TYPE_H_