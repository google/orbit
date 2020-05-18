// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <utility>

#include "OrbitSsh/Context.h"

namespace OrbitSsh {

TEST(Context, Create) {
  auto context = Context::Create();
  ASSERT_TRUE(context.has_value());
  ASSERT_TRUE(context.value().isActiveContext());

  auto context2 = std::move(context.value());
  ASSERT_FALSE(context.value().isActiveContext());
  ASSERT_TRUE(context2.isActiveContext());
}
}  // namespace OrbitSsh
