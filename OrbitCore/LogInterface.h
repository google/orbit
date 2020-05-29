/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <string>
#include <vector>

class LogInterface {
 public:
  static std::vector<std::string> GetOutput();
};