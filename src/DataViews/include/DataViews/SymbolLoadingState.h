// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_SYMBOL_LOADING_STATE_H_
#define DATA_VIEWS_SYMBOL_LOADING_STATE_H_

#include <absl/base/attributes.h>

#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_data_views {

// Class to represent the current state of symbol loading for a certain module. Also provides a
// textual description for each state via GetName and a color via GetDisplayColor
struct SymbolLoadingState {
  // TODO(b/202140068) remove unknown when not needed anymore
  enum State { kUnknown, kDisabled, kDownloading, kError, kLoading, kLoaded, kFallback } state;
  SymbolLoadingState(State initial_state) : state(initial_state) {}

  [[nodiscard]] std::string GetName() const {
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
      case kFallback:
        return "Partial";
    }
    ORBIT_UNREACHABLE();
  }

  [[nodiscard]] std::string GetDescription() const {
    switch (state) {
      case kUnknown:
        return "";
      case kDisabled:
        return "Loading symbols automatically is always disabled for this module.";
      case kDownloading:
        return "A file containing symbol information for this module has been found and is being "
               "downloaded.";
      case kError:
        return "No symbols could be found for this module.";
      case kLoading:
        return "Symbols for this module are now being loaded from a file.";
      case kLoaded:
        return "Debug symbols for this module have been loaded successfully.";
      case kFallback:
        return "No debug symbols could be found for this module.\n"
               "Nonetheless, some substitute information could still be extracted from the module "
               "itself,\n"
               "namely from symbols for dynamic linking and/or from stack unwinding "
               "information.\n"
               "\n"
               "Note that this information might be inaccurate.";
    }
    ORBIT_UNREACHABLE();
  }

  // Returns false for the default color and true for any other color, in the latter case the
  // parameters are changed.
  bool GetDisplayColor(unsigned char& red, unsigned char& green, unsigned char& blue) const {
    switch (state) {
      case kUnknown:
      case kLoaded:
        return false;
      case kDisabled: {
        red = 153;
        green = 153;
        blue = 153;
        break;
      }
      case kDownloading:
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
      case kFallback: {
        red = 230;
        green = 150;
        blue = 70;
        break;
      }
    }
    return true;
  }
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_SYMBOL_LOADING_STATE_H_