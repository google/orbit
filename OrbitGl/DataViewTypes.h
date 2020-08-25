// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_VIEW_TYPES_H_
#define ORBIT_GL_DATA_VIEW_TYPES_H_

enum class DataViewType {
  kInvalid,
  kFunctions,
  kLiveFunctions,
  kCallstack,
  kProcesses,
  kModules,
  kSampling,
  kPresets,
  kTracepoints,
  kAll,

};

#endif  // ORBIT_GL_DATA_VIEW_TYPES_H_
