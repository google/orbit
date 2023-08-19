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
  kAll,  // kAll needs to be last.
};

inline const char* ToString(DataViewType type) {
  switch (type) {
    case DataViewType::kInvalid:
      return "kInvalid";
    case DataViewType::kFunctions:
      return "kFunctions";
    case DataViewType::kLiveFunctions:
      return "kLiveFunctions";
    case DataViewType::kCallstack:
      return "kCallstack";
    case DataViewType::kModules:
      return "kModules";
    case DataViewType::kSampling:
      return "kSampling";
    case DataViewType::kPresets:
      return "kPresets";
    case DataViewType::kTracepoints:
      return "kTracepoints";
    case DataViewType::kAll:
      return "kAll";
  }
  ORBIT_UNREACHABLE();
  return "";
}

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_TYPE_H_