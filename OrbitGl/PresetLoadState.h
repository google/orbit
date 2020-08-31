// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESET_LOAD_STATE_H_
#define ORBIT_GL_PRESET_LOAD_STATE_H_

struct PresetLoadState {
  enum State { kLoadable, kPartiallyLoadable, kNotLoadable } state;
  PresetLoadState(State initial_state) : state(initial_state){};

  [[nodiscard]] inline std::string GetName() const {
    switch (state) {
      case PresetLoadState::kLoadable:
        return "Yes";
      case PresetLoadState::kPartiallyLoadable:
        return "Partially";
      case PresetLoadState::kNotLoadable:
        return "No";
    }
    return "";
  }
};

#endif  // ORBIT_GL_PRESET_LOAD_STATE_H_