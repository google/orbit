// TODO(b/148520406): Add copyright here
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <gtest/gtest.h>

#include "Path.h"
#include "absl/strings/match.h"

TEST(Path, GetSourceRoot) {
  // On windows GetSroucePath converts '\' to '/'
  // to account for that call GetDirectory
  ASSERT_TRUE(absl::StartsWith(Path::GetDirectory(__FILE__),
                               Path::GetSourceRoot()));
}

