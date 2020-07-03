// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PARAMS_H_
#define ORBIT_CORE_PARAMS_H_

struct Params {
  Params();

 public:
  const float font_size = 14.f;
};

extern Params GParams;

#endif  // ORBIT_CORE_PARAMS_H_
