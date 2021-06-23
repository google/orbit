// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(b/185090791): Remove this file and use orbit_data_views::CompareAscendingOrDescending
//  instead once all DataViews have been moved into the DataViews module.

#ifndef ORBIT_GL_COMPARE_ASCENDING_OR_DESCENDING_H_
#define ORBIT_GL_COMPARE_ASCENDING_OR_DESCENDING_H_

namespace orbit_gl {

template <class T>
inline bool CompareAscendingOrDescending(const T& a, const T& b, bool ascending) {
  return ascending ? a < b : a > b;
}

}  // namespace orbit_gl

#endif  // ORBIT_GL_COMPARE_ASCENDING_OR_DESCENDING_H_
