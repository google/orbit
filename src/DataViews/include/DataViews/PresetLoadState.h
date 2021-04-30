// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_PRESET_LOAD_STATE_H_
#define DATA_VIEWS_PRESET_LOAD_STATE_H_

#include <string>

namespace orbit_data_views {

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

  inline void GetDisplayColor(unsigned char& red, unsigned char& green, unsigned char& blue) const {
    switch (state) {
      case PresetLoadState::kLoadable:
        red = 200;
        green = 240;
        blue = 200;
        break;
      case PresetLoadState::kPartiallyLoadable:
        red = 240;
        green = 240;
        blue = 200;
        break;
      case PresetLoadState::kNotLoadable:
        red = 230;
        green = 190;
        blue = 190;
        break;
    }
  }
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_PRESET_LOAD_STATE_H_