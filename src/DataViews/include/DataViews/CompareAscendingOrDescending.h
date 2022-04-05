// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_COMPARE_ASCENDING_OR_DESCENDING_H_
#define DATA_VIEWS_COMPARE_ASCENDING_OR_DESCENDING_H_

namespace orbit_data_views_internal {

template <class T>
inline bool CompareAscendingOrDescending(const T& a, const T& b, bool asc) {
  return asc ? a < b : a > b;
}

}  // namespace orbit_data_views_internal

#endif  // DATA_VIEWS_COMPARE_ASCENDING_OR_DESCENDING_H_
