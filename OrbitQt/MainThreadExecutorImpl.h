// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_
#define ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_

#include "MainThreadExecutor.h"

// Create main thread executor implementation based on QT event loop.
std::unique_ptr<MainThreadExecutor> CreateMainThreadExecutor();

#endif  // ORBIT_QT_MAIN_THREAD_EXECUTOR_IMPL_H_
