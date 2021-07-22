// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Context.h"

namespace orbit_ssh {

TEST(Context, Create) {
  auto context = Context::Create();
  ASSERT_TRUE(context);
  ASSERT_TRUE(context.value().active());

  auto context2 = std::move(context.value());
  ASSERT_FALSE(context.value().active());
  ASSERT_TRUE(context2.active());
}
}  // namespace orbit_ssh
