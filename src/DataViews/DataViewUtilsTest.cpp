// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include <string>

#include "DataViewUtils.h"

#ifdef _WIN32
#include <windows.h>

static void SetEnv(const char* name, const char* value) { _putenv_s(name, value); }
#else
#include <stdlib.h>

static void SetEnv(const char* name, const char* value) { setenv(name, value, 1); }
#endif

namespace orbit_data_views {

TEST(DataViewUtils, FormatShortDatetime) {
  // `FormatShortDateTime` outputs the time in local time zone.
  // Setting `TZ` ensure the time zone will always be `UTC`.
  SetEnv("TZ", "UTC");

  absl::Time datetime = absl::FromTimeT(0);
  std::string result = FormatShortDatetime(datetime);
  EXPECT_EQ(result, "01/01/1970 00:00 AM");
}

}  // namespace orbit_data_views