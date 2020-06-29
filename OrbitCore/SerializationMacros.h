// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#define ORBIT_SERIALIZABLE \
  template <class Archive> \
  void serialize(Archive& a_Archive, std::uint32_t const a_Version)
