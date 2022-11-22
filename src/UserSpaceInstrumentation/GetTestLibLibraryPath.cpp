// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>
#include <string>
#include <string_view>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

ErrorMessageOr<std::filesystem::path> GetTestLibLibraryPath() {
  // copybara:strip_begin(The library is in a different place internally)
  constexpr std::string_view kLibName = "libUserSpaceInstrumentationTestLib.so";
  const std::string library_path = orbit_base::GetExecutableDir() / ".." / "lib" / kLibName;
  /* copybara:strip_end_and_replace
  const std::string library_path = "@@LIB_USER_SPACE_INSTRUMENTATION_TEST_LIB_PATH@@";
  */

  OUTCOME_TRY(orbit_base::OpenFileForReading(library_path));

  return library_path;
}

}  // namespace orbit_user_space_instrumentation