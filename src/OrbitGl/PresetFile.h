// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESET_FILE_H_
#define ORBIT_GL_PRESET_FILE_H_

#include "preset.pb.h"

namespace orbit_gl {

struct PresetFile {
  std::filesystem::path file_path;
  orbit_client_protos::PresetInfo preset_info;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PRESET_FILE_H_
