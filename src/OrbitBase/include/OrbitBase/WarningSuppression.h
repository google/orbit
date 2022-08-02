// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_WARNING_SUPPRESSION_H_
#define ORBIT_BASE_WARNING_SUPPRESSION_H_

#ifdef _WIN32

// Disabling "destructor was implicitly defined as deleted" warning when including LLVM headers
// (b/241025245).
#define ORBIT_LLVM_INCLUDE_BEGIN _Pragma("warning(push)") _Pragma("warning(disable : 4624)")
#define ORBIT_LLVM_INCLUDE_END _Pragma("warning(pop)")

#else

#define ORBIT_LLVM_INCLUDE_BEGIN
#define ORBIT_LLVM_INCLUDE_END

#endif

#endif  // ORBIT_BASE_WARNING_SUPPRESSION_H_