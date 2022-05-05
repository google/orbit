// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_SYMBOL_LOADING_STATE_H_
#define DATA_VIEWS_SYMBOL_LOADING_STATE_H_

#include <absl/base/attributes.h>

#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_data_views {

struct SymbolLoadingState {
  // TODO(b/202140068) remove unknown when not needed anymore
  enum State { kUnknown, kDisabled, kDownloading, kError, kLoading, kLoaded } state;
  SymbolLoadingState(State initial_state) : state(initial_state) {}

  [[nodiscard]] std::string GetDescription() const {
    switch (state) {
      case kUnknown:
        return "";
      case kDisabled:
        return "Disabled";
      case kDownloading:
        return "Downloading...";
      case kError:
        return "Error";
      case kLoading:
        return "Loading...";
      case kLoaded:
        return "Loaded";
    }
    ORBIT_UNREACHABLE();
  }

  // Returns false for the default color and true for any other color, in the latter case the
  // parameters are changed.
  bool GetDisplayColor(unsigned char& red, unsigned char& green, unsigned char& blue) const {
    switch (state) {
      case kUnknown:
        ABSL_FALLTHROUGH_INTENDED;
      case kLoaded:
        return false;
      case kDisabled: {
        red = 153;
        green = 153;
        blue = 153;
        break;
      }
      case kDownloading:
        ABSL_FALLTHROUGH_INTENDED;
      case kLoading: {
        red = 55;
        green = 138;
        blue = 221;
        break;
      }
      case kError: {
        red = 230;
        green = 70;
        blue = 70;
        break;
      }
    }
    return true;
  }
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_SYMBOL_LOADING_STATE_H_