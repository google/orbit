// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ATTRIBUTES_H_
#define ORBIT_BASE_ATTRIBUTES_H_

// GCC and Clang both define "__has_attribute(x)", which evaluates to 1 if the attribute is
// supported by the current compilation target. Define a version of "__has_attribute(x)" that
// evaluates to 0 for other compilers.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

// Generate code without prolog and epilog. We expect the compiler to support the "naked" attribute
// and we generate a compilation error otherwise by not providing an empty default.
#if __has_attribute(naked)
#define ORBIT_NAKED __attribute__((naked))
#elif _WIN32
#define ORBIT_NAKED __declspec(naked)
#endif

// Prevent inlining of function. We expect the compiler to support the "noinline" attribute and we
// generate a compilation error otherwise by not providing an empty default.
#if __has_attribute(noinline)
#define ORBIT_NOINLINE __attribute__((noinline))
#elif _WIN32
#define ORBIT_NOINLINE __declspec(noinline)
#endif

#endif  // ORBIT_BASE_ATTRIBUTES_H_
