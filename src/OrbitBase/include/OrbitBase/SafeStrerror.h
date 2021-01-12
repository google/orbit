// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SAFE_STRERROR_H_
#define ORBIT_BASE_SAFE_STRERROR_H_

// strerror is not thread safe, as the implementation could reuse a buffer that
// is not thread local and can be modified by a subsequent call to strerror or
// strerror_l. This function instead uses a thread_local buffer with strerror_r
// (or C11 strerror_s with MSVC).
const char* SafeStrerror(int errnum);

#endif  // ORBIT_BASE_SAFE_STRERROR_H_
