// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Error.h"

#include <absl/strings/str_format.h>

namespace OrbitGgp {

std::string ErrorCategory::message(int condition) const {
  switch (static_cast<Error>(condition)) {
    case Error::kCouldNotUseGgpCli:
      return "Unable to use the ggp command line interface.";
    case Error::kGgpListInstancesFailed:
      return "Listing available instances failed.";
    case Error::kRequestTimedOut:
      return "Request timed out.";
    case Error::kUnableToParseJson:
      return "Unable to parse JSON.";
  }

  return absl::StrFormat("Unkown error condition: %i.", condition);
}

}  // namespace OrbitGgp