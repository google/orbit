// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_INTERNAL_IDENTITY_H_
#define ORBIT_BASE_INTERNAL_IDENTITY_H_

namespace internal {

template <typename T>
struct identity {
  typedef T type;
};

};  // namespace internal

#endif  // ORBIT_BASE_INTERNAL_IDENTITY_H_
